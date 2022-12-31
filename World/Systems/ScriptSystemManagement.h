#pragma once
#include <Application.h>
#include <World/Components/ScriptComponent.h>
#include <unordered_map>
#include <LuaEngineUtilities.h>
#include <World/System.h>
#include <World/Entity.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <unordered_set>
#include <LuaEngine.h>

class ScriptSystemVM;
class ScriptHandler;

struct Script_Variant_Key_Value {
    Script_Variant_Key_Value(const Script_Variant_type& value,const std::string& name) : value(value), name(name) {}
    
    Script_Variant_type value;
    std::string name;
};

using Deffered_Set_Map = std::unordered_map<uint32_t, std::vector<Script_Variant_Key_Value>>;

class ScriptObject {
public:
    ScriptObject(const std::string& ref) : script(ref) {}
    std::string script;
};

class ScriptSystemManager {
public:
    
    ScriptSystemManager(const ScriptSystemManager& ref) = delete;
    ScriptSystemManager(ScriptSystemManager&& ref) = delete;
    ScriptSystemManager& operator=(const ScriptSystemManager& ref) = delete;
    ScriptSystemManager& operator=(ScriptSystemManager&& ref) = delete;

    static void Initialize();
    static ScriptSystemManager* Get();
    static void Shutdown();

    void InitThread();

    std::string& GetScript(const std::string& path);

    std::string& GetConstructionScript(const std::string& path);

    void UploadScript(const std::string& path, const std::string& script);

    void UploadConstructionScript(const std::string& path, const std::string& script);

    void SetEntityAsDirty(Entity ent);

    //This is not ThreadSafe, use only in synchronized contexts.
    void InvalidateInlineScript(const std::string& script_path);


    //This is not ThreadSafe, use only in synchronized contexts.
    void InvalidateConstructionScript(const std::string& script_path);

    const std::vector<Deffered_Set_Map>& GetEntityChanges();

    void ClearEntityChanges();

    ScriptSystemVM* TryGetScriptSystemVM();

    void InitializeScriptSystemVM();
    
    //Only call from main thread when vms aren't used, this call is NOT thread-safe
    void ResetAllScriptSystemVMs();

private:
    static ScriptSystemManager* instance;
    ~ScriptSystemManager();
    ScriptSystemManager();

    Deffered_Set_Map* GetDefferedSetMap();

private:

    std::unordered_map<std::string, ScriptObject> m_ScriptCache;
    std::vector<ScriptSystemVM*> m_Script_system_VMs;
    std::mutex sync_mutex;
    std::mutex script_cache_mutex;

    std::mutex DefferedSetMaps_mutex;
    std::vector<Deffered_Set_Map> m_DefferedSetMaps;
};

#pragma region ScriptHandler

class ScriptHandler {
public:
    ScriptHandler(Entity ent) : current_entity(ent) {}
    static void BindHandlerFunctions(LuaEngineClass<ScriptHandler>* script_engine);
    static void BindKeyCodes(LuaEngineClass<ScriptHandler>* script_engine);
    //This is where Functions which are bound to Lua go
#pragma region LuaBound
    glm::vec2 TestGetPosition();

    template<typename T>
    T GetProperty(std::string name) {
        auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(current_entity).m_Properties;
        auto find = props.find(name);
        if (find != props.end()) {
            Script_Variant_type& prop = (*find).second;
            try {

                T& value = std::get<T>(prop);
                return value;

            } catch(std::bad_variant_access& e){
                //std::cout << e.what() << "\n";
                throw std::runtime_error("Invalid Property Access");
            }
        }
        else {
            throw std::runtime_error("Property " + name + " was not found");
        }
    }

    template<typename T>
    void SetProperty(std::string name,T value) {
        auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(current_entity).m_Properties;
        auto find = props.find(name);
        if (find != props.end()) {
            Script_Variant_type& prop = (*find).second;
            if (std::get_if<T>(&prop)) {
                prop.emplace<T>(value);
                return;
            }
            else {
                throw std::runtime_error("Invalid Property Access");
            }
        }
        else {
            props.insert(std::make_pair(name, value));
            return;
        }
    }

    bool PropertyExists(std::string name);

    Entity GetEntity() const {
        return current_entity;
    }


#pragma endregion
private:
    Entity current_entity;
};

#pragma endregion

#pragma region InitializationScriptHandler

class InitializationScriptHandler {
public:
    InitializationScriptHandler(Entity ent, const std::string& path) : current_entity(ent), current_path(path) {}
    static void BindHandlerFunctions(LuaEngineClass<InitializationScriptHandler>* script_engine);
    //This is where Functions which are bound to Lua go
#pragma region LuaBound
    
    template<typename T, typename ... Args>
    void SetComponent(Args&&... args) {
        Application::GetWorld().SetComponent<T>(current_entity, T(std::forward<Args>(args)...));
    }

    void SetScriptComponent(std::string path);

    void SetSquareComponent(glm::vec4 color = glm::vec4(1.0f));

    void SetCameraComponent(float fov, float zNear, float zFar, float aspect_ratio);

    void SetTranslation(glm::vec3 translation);

    void SetScale(glm::vec3 scale);

    void EnableKeyPressedEvents();

    void EnableMouseButtonPressedEvents();

    Entity GetEntity() const {
        return current_entity;
    }

#pragma endregion
private:
    std::string current_path;
    Entity current_entity;
};

#pragma endregion


class ScriptSystemVM {
public:
    friend ScriptSystemManager;
private:
    ScriptSystemVM();
    ~ScriptSystemVM();
public:
    void SetEngineEntity(Entity ent);

    Entity GetCurrentEntity() const {
        if (init_mode) {
            return current_Initialization_handler.GetEntity();
        }
        else {
            return curentHandler.GetEntity();
        }
    }

    Entity GetEngineEntity() const {
        return curentHandler.GetEntity();
    }

    // For Script Runtime

    template<typename R, typename ... Args>
    auto CallFunction(const std::string& path,const std::string& function_name, Args ... args) 
        -> std::enable_if_t<(!std::is_void_v<R>),R>
    {
        R out;
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObject<void>(nullptr, LuaEngineUtilities::ScriptHash(path), function_name, args...);
            if (success) {
                return out;
            }
            else {
                throw std::runtime_error("An error occured in a ScriptVM lua function execution");
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetScript(path);
            m_LuaEngine.RunString(script);
            m_BoundScripts.insert(LuaEngineUtilities::ScriptHash(path));
            return CallFunction<R>(path, function_name, args...);
        }
    }

    template<typename R, typename ... Args>
    auto TryCallFunction(R* out,const std::string& path, const std::string& function_name, Args ... args)
        -> std::enable_if_t<(!std::is_void_v<R>), bool>
    {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObject<void>(out, LuaEngineUtilities::ScriptHash(path), function_name, args...);
            if (success) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetScript(path);
            m_LuaEngine.RunString(script);
            m_BoundScripts.insert(LuaEngineUtilities::ScriptHash(path));
            return TryCallFunction(out, path, function_name, args...);
        }
    }

    template<typename ... Args>
    void CallFunction(const std::string& path, const std::string& function_name, Args ... args) {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success;
            {
                success = m_LuaEngine.TryCallObject<void>(nullptr,LuaEngineUtilities::ScriptHash(path).c_str(), function_name.c_str(), args...);
            }
            if (success) {
                return;
            }
            else {
                throw std::runtime_error("An error occured in a ScriptVM lua function execution");
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetScript(path);
            m_LuaEngine.RunString(script);
            m_BoundScripts.insert(LuaEngineUtilities::ScriptHash(path));
            CallFunction(path, function_name, args...);
        }
    }

    template<typename ... Args>
    bool TryCallFunction(void* null,const std::string& path, const std::string& function_name, Args ... args) {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success;
            {
                success = m_LuaEngine.TryCallObject<void>(nullptr, LuaEngineUtilities::ScriptHash(path).c_str(), function_name.c_str(), args...);
            }
            if (success) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetScript(path);
            m_LuaEngine.RunString(script);
            m_BoundScripts.insert(LuaEngineUtilities::ScriptHash(path));
            return TryCallFunction(nullptr, path, function_name, args...);
        }
    }

    // For Initialization Scripts.

    void SetEngineInitializationEntity(Entity ent, const std::string& path);

    Entity GetEngineInitializationEntity() const {
        return current_Initialization_handler.GetEntity();
    };

    template<typename R, typename ... Args>
    auto CallInitializationFunction(const std::string& path, const std::string& function_name, Args ... args)
        -> std::enable_if_t<(!std::is_void_v<R>), R>
    {
        R out;
        if (m_BoundInitializationScripts.find(LuaEngineUtilities::ScriptHash(path, true)) != m_BoundInitializationScripts.end()) {
            bool success = m_LuaInitializationEngine.TryCallObject<void>(nullptr, LuaEngineUtilities::ScriptHash(path, true), function_name, args...);
            if (success) {
                return out;
            }
            else {
                throw std::runtime_error("An error occured in a ScriptVM lua function execution");
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetConstructionScript(path);
            m_LuaInitializationEngine.RunString(script);
            m_BoundInitializationScripts.insert(LuaEngineUtilities::ScriptHash(path, true));
            return CallInitializationFunction<R>(path, function_name, args...);
        }
    }

    template<typename R, typename ... Args>
    auto TryCallInitializationFunction(R* out, const std::string& path, const std::string& function_name, Args ... args)
        -> std::enable_if_t<(!std::is_void_v<R>), bool>
    {
        if (m_BoundInitializationScripts.find(LuaEngineUtilities::ScriptHash(path, true)) != m_BoundInitializationScripts.end()) {
            bool success = m_LuaInitializationEngine.TryCallObject<void>(out, LuaEngineUtilities::ScriptHash(path, true), function_name, args...);
            if (success) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetConstructionScript(path);
            m_LuaInitializationEngine.RunString(script);
            m_BoundInitializationScripts.insert(LuaEngineUtilities::ScriptHash(path, true));
            return TryCallInitializationFunction(out, path, function_name, args...);
        }
    }

    template<typename ... Args>
    void CallInitializationFunction(const std::string& file_path, const std::string& function_name, Args ... args) {
        auto path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
        if (m_BoundInitializationScripts.find(LuaEngineUtilities::ScriptHash(path, true)) != m_BoundInitializationScripts.end()) {
            bool success;
            {
                success = m_LuaInitializationEngine.TryCallObject<void>(nullptr, LuaEngineUtilities::ScriptHash(path,true).c_str(), function_name.c_str(), args...);
            }
            if (success) {
                return;
            }
            else {
                throw std::runtime_error("An error occured in a ScriptVM lua function execution");
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetConstructionScript(path);
            m_LuaInitializationEngine.RunString(script);
            m_BoundInitializationScripts.insert(LuaEngineUtilities::ScriptHash(path, true));
            CallInitializationFunction(path, function_name, args...);
        }
    }

    template<typename ... Args>
    bool TryCallInitializationFunction(void* null, const std::string& path, const std::string& function_name, Args ... args) {
        if (m_BoundInitializationScripts.find(LuaEngineUtilities::ScriptHash(path, true)) != m_BoundInitializationScripts.end()) {
            bool success;
            {
                success = m_LuaInitializationEngine.TryCallObject<void>(nullptr, LuaEngineUtilities::ScriptHash(path, true).c_str(), function_name.c_str(), args...);
            }
            if (success) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            std::string script = ScriptSystemManager::Get()->GetConstructionScript(path);
            m_LuaInitializationEngine.RunString(script);
            m_BoundInitializationScripts.insert(LuaEngineUtilities::ScriptHash(path, true));
            return TryCallInitializationFunction(nullptr, path, function_name, args...);
        }
    }

    void InvalidateInlineScript(const std::string& script_path);
    void InvalidateConstructionScript(const std::string& script_path);

    void ResetScriptVM();


private:
    ScriptHandler curentHandler;
    bool init_mode = false;

    std::unordered_set<std::string> m_BoundScripts;
    LuaEngineClass<ScriptHandler> m_LuaEngine;

    InitializationScriptHandler current_Initialization_handler;

    std::unordered_set<std::string> m_BoundInitializationScripts;  
    LuaEngineClass<InitializationScriptHandler> m_LuaInitializationEngine;
};

