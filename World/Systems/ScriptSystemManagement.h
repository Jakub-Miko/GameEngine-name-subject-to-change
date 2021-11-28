#pragma once
#include <World/Components/ScriptComponent.h>
#include <unordered_map>
#include <LuaEngineUtilities.h>
#include <World/System.h>
#include <World/Entity.h>
#include <unordered_set>
#include <LuaEngine.h>

class ScriptSystemVM;
class ScriptHandler;

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

    std::string& GetScript(const std::string& path);

    ScriptSystemVM* TryGetScriptSystemVM();

    void InitializeScriptSystemVM();
    
    //Only call from main thread when vms aren't used, this call is NOT thread-safe
    void ResetAllScriptSystemVMs();

private:
    static ScriptSystemManager* instance;
    ~ScriptSystemManager();
    ScriptSystemManager();

private:
    
    std::unordered_map<std::string, ScriptObject> m_ScriptCache;
    std::vector<ScriptSystemVM*> m_Script_system_VMs;
    std::mutex sync_mutex;
};

#pragma region ScriptHandler

class ScriptHandler {
public:
    ScriptHandler(Entity ent) : current_entity(ent) {}
    static void BindHandlerFunctions(LuaEngineClass<ScriptHandler>* script_engine);
    static void BindKeyCodes(LuaEngineClass<ScriptHandler>* script_engine);
    //This is where Functions which are bound to Lua go
#pragma region LuaBound
    void TestChangeSquarePos(float x, float y);
    glm::vec2 TestGetPosition();

    template<typename T>
    T GetProperty(std::string name) {
        auto& props = Application::GetWorld().GetComponent<ScriptComponent>(current_entity).m_Properties;
        auto find = props.find(name);
        if (find != props.end()) {
            Script_Variant_type& prop = (*find).second;
            try {

                T& value = std::get<T>(prop);
                return value;

            } catch(std::bad_variant_access& e){
                std::cout << e.what() << "\n";
                throw std::runtime_error("Invalid Property Access");
            }
        }
        else {
            throw std::runtime_error("Property " + name + " was not found");
        }
    }

    template<typename T>
    void SetProperty(std::string name,T value) {
        auto& props = Application::GetWorld().GetComponent<ScriptComponent>(current_entity).m_Properties;
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

    bool IsKeyPressed(int key_code);

    bool IsMouseButtonPressed(int key_code);

    glm::vec2 GetMousePosition();

    void EnableKeyPressedEvents();

    void EnableMouseButtonPressedEvents();

#pragma endregion
private:
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

    template<typename R, typename ... Args>
    auto CallFunction(const std::string& path,const std::string& function_name, Args ... args) 
        -> std::enable_if_t<(!std::is_void_v<R>),R>
    {
        R out;
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObject<void>(nullptr, LuaEngineUtilities::ScriptHash(path), function_name, args...);
            if (success) {
                return R;
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
            TryCallFunction(out, path, function_name, args...);
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
            TryCallFunction(nullptr, path, function_name, args...);
        }
    }

    void ResetScriptVM();


private:
    ScriptHandler curentHandler;
    std::unordered_set<std::string> m_BoundScripts;
    LuaEngineClass<ScriptHandler> m_LuaEngine;
};
