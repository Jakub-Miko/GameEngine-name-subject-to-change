#pragma once
#include <FileManager.h>
#include <Application.h>
#include <Events/SubjectObserver.h>
#include <World/Components/ScriptComponent.h>
#include <unordered_map>
#include <LuaEngineUtilities.h>
#include <World/System.h>
#include <World/Entity.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/ScriptModules/CollisionModule.h>
#include <unordered_set>
#include <LuaEngine.h>

class ScriptSystemVM;
class ScriptHandler;
class CollisionEvent;

/**
 * @brief Key-Value pair of a Script value stored as a variant
 * 
 * Used to store @ref DynamicPropertiesComponent "property" set actions which have been deffered @see ScriptSystemDefferedSet
*/
struct Script_Variant_Key_Value {
    Script_Variant_Key_Value(const Script_Variant_type& value,const std::string& name) : value(value), name(name) {}
    
    Script_Variant_type value; ///< The value of a property stored as a variant
    std::string name; ///< The name of the property 
};

/**
 * @brief A map containing associations between entities and all @ref Script_Variant_Key_Value "Script_Variant_Key_Values" which should be set in its DynamicPropertiesComponent during @ref ScriptSystemDefferedSet
*/
using Deffered_Set_Map = std::unordered_map<uint32_t, std::vector<Script_Variant_Key_Value>>;

/**
 * @brief A script function name  and its arguments, which should be called during @ref ScriptSystemDefferedCall for a deffered function call 
*/
struct Deffered_Call {
    std::string func_name;
    std::vector<Script_Variant_type> arguments;
};

/**
* @brief A map containing associations between entities and all @ref Deffered_Call "deffered calls" which should be called during @ref ScriptSystemDefferedCall
*/
using Deffered_Call_Map = std::unordered_map<uint32_t, std::vector<Deffered_Call>>;

/**
 * @brief An object containing a lua script string used as a cache entry to be loaded to all lua VMs which need it without reopening and parsing files
*/
class ScriptObject {
public:
    ScriptObject(const std::string& ref) : script(ref) {}
    std::string script; ///< The lua script loaded from a file
};

/**
 * @brief A singleton that manages a multithreaded Lua execution environment, which allows for calling various scripts for all entities in parallel, despite lua not supporting multithreading.
*/
class ScriptSystemManager {
public:
    
    ScriptSystemManager(const ScriptSystemManager& ref) = delete;
    ScriptSystemManager(ScriptSystemManager&& ref) = delete;
    ScriptSystemManager& operator=(const ScriptSystemManager& ref) = delete;
    ScriptSystemManager& operator=(ScriptSystemManager&& ref) = delete;

    /**
     * @brief Singleton initializer
    */
    static void Initialize();
    /**
     * @brief Singleton getter
    */
    static ScriptSystemManager* Get();
    /**
     * @brief Singleton destructor
    */
    static void Shutdown();

    /**
     * @brief Called in every thread which will run lua scripts using the ScriptSystemManager to initialize necessary threadlocal data. 
    */
    void InitThread();

    /**
     * @brief Gets a string containing a an @ref inline_script "inline script" loaded from a file, with caching
     * @param path Path to the file containing the @ref inline_script "inline script", allows file subsections
     * @return a string containing the loaded @ref inline_script "inline script"
    */
    std::string& GetScript(const std::string& path);

    /**
     * @brief Gets a string containing a a @ref construction_script "construction script" loaded from a file, with caching
     * @param path Path to the file containing the @ref construction_script "construction script", allows file subsections
     * @return a string containing the loaded @ref construction_script "construction script"
    */
    std::string& GetConstructionScript(const std::string& path);

    /**
     * @brief Used to replace a cache entry of an @ref inline_script "inline script" loaded from a file, with a script contained in a string
     * @param path path of the @ref inline_script "inline script" cache entry to replace
     * @param script the script to replace the @ref inline_script "inline script" cache entry with
    */
    void UploadScript(const std::string& path, const std::string& script);

    /**
     * @brief Used to replace a cache entry of a @ref construction_script "construction script" loaded from a file, with a script contained in a string
     * @param path path of the @ref construction_script "construction script" cache entry to replace
     * @param script the script to replace the @ref construction_script "construction script" cache entry with
    */
    void UploadConstructionScript(const std::string& path, const std::string& script);

    /**
     * @brief Marks entities which have new pending @ref ScriptSystemDefferedSet "deffered property set actions" as dirty by assigning them wit a DefferedUpdateComponent
     * @param ent Entity to mark dirty 
    */
    void SetEntityAsDirty(Entity ent);

    /**
     * @brief Get the Deffered_Call_Map containg pending deffered calls @see ScriptSystemDefferedCall 
     * @return the current Deffered_Call_Map
     * 
     * @note The current deffered map alternates on every internal iteration of ScriptSystemDefferedCall, so deffered calls can also spawn other deffered calls
    */
    Deffered_Call_Map& GetDefferedCalls();
    /**
     * @brief Gets a vector contaning all @ref Deffered_Call "deffered calls" pending for an Entity.
     * @param ent Entity to get the @ref Deffered_Call "deffered calls" for 
     * @return all @ref Deffered_Call "deffered calls" pending for an Entity
    */
    std::vector<Deffered_Call>& GetDefferedCallsForEntity(Entity ent);
    /**
     * @brief Get all Entities with pending deffered calls
     * @return vector of entities with pending deffered calls
    */
    std::vector<Entity>& GetPendingDefferedCallEntities();
    /**
     * @brief Add a new deffered call to an Entity
     * @param ent Entity to add the deffered call to (the one on which it will be called, not the one which added it)
     * @param call_info Deffered_Call object containing the function name and arguments
    */
    void AddDefferedCall(Entity ent, const Deffered_Call& call_info);

    /**
     * @brief Used by @ref ScriptSystemDefferedCall to swap Deffered call maps, so that deffered calls can also spawn other deffered calls. @see ScriptSystemManager::GetDefferedCalls
    */
    void SwapDefferedCallCycle();

    
    //This is not ThreadSafe, use only in synchronized contexts.
    
    /**
     * @brief Erases an @ref inline_script "inline script" from all lua VMs and the cache, to force a reload from file on next call
     * @param script_path Path to the script to invalidate
     * 
     * @warning This is not thread-safe, use only in synchronized contexts
    */
    void InvalidateInlineScript(const std::string& script_path);
    
    /**
     * @brief Erases a @ref construction_script "construction script" from all lua VMs and the cache, to force a reload from file on next call
     * @param script_path Path to the script to invalidate
     *
     * @warning This is not thread-safe, use only in synchronized contexts
    */
    void InvalidateConstructionScript(const std::string& script_path);

    /**
     * @brief Gets a vector of maps containing @ref ScriptSystemDefferedSet "deffered property set actions", one map for every thread.
     * @return a vector of @ref Deffered_Set_Map "Deffered_Set_Maps" for every thread
    */
    const std::vector<Deffered_Set_Map>& GetEntityChanges();

    /**
     * @brief Celars all @ref Deffered_Set_Map "Deffered_Set_Maps" for every thread
    */
    void ClearEntityChanges();

    /**
     * @brief Gets a ScriptSystemVM for executing script
     * @return a ScriptSystemVM instance if available, nullptr if not
     * 
     * The ScriptSystemVM returned is specific to the thread this function was called on, and the thread must have called ScriptSystemManager::InitializeScriptSystemVM, otherwise
     * nullptr is returned
    */
    ScriptSystemVM* TryGetScriptSystemVM();

    /**
     * @brief Registers an entity collision which will result in a call to OnCollision script functions call. @see ScriptSystemCollisionCallback
     * @param col_event CollisionEvent launched by the PhysicsEngine
    */
    void OnCollision(CollisionEvent* col_event);

    /**
     * @brief Called by every thread utilizing ScriptSystemManager, to initialize a thread local ScriptSystemVM instance
    */
    void InitializeScriptSystemVM();

    /**
     * @brief Resets all ScriptSystemVM instances to their default state
     * @warning Only call from main thread when vms aren't used, this call is NOT thread-safe
    */
    void ResetAllScriptSystemVMs();

    /**
     * @brief Get all entities which have experienced collisions and have PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS flag set
     * @return the list of collided entities with callbacks enabled
    */
    std::vector<Entity>& GetCollidedEntities() {
        return collided_entities; /// @todo returning mutable references here might not be the best idea
    }

    /**
     * @brief Gets all collision events, associated with all entities
     * @return a maps between entities and vectors of collision events
    */
    std::unordered_map<uint32_t, std::vector<CollisionEvent_L>>& GetEntityCollisions() {
        return entity_collisions; /// @todo returning mutable references here might not be the best idea
    }

    /**
     * @brief Gets all collision events, associated with an Entity
     * @param ent Entity to get the collisions of
     * @return a vector of all collision events for an Entity
    */
    std::vector<CollisionEvent_L>& GetEntityCollisions(Entity ent);


private:
    static ScriptSystemManager* instance;
    ~ScriptSystemManager();
    ScriptSystemManager();

    /**
     * @brief Internal function that creates and return a new Deffered_Set_Map
     * @return a new instance of Deffered_Set_Map
     * 
     * @warning Only as many @ref Deffered_Set_Map "Deffered_Set_Maps" can be allocated as @ref ThreadManager::GetMaxThreadCount "Max Thread Count"
    */
    Deffered_Set_Map* GetDefferedSetMap();

private:

    std::unordered_map<std::string, ScriptObject> m_ScriptCache; ///< Cache associating script filepaths with loaded scripts
    std::vector<ScriptSystemVM*> m_Script_system_VMs; ///< a vector containing all @ref ScriptVM "ScriptVMs" used by all threads utilizing ScriptSystemManager
    std::mutex sync_mutex; ///< Mutex for @ref m_Script_system_VMs (Only used during @ref ScriptSystemManager::InitializeScriptSystemVM "thread initialization", since no writes are performed otherwise)
    std::mutex script_cache_mutex; ///< Mutex for @ref m_ScriptCache

    std::mutex DefferedSetMaps_mutex; ///< Mutex for @ref m_DefferedSetMaps
    std::vector<Deffered_Set_Map> m_DefferedSetMaps; ///< Vector of @ref Deffered_Set_Map "Deffered_Set_Maps" for all threads

    //Deffered calls have different cycles one used for reads and one for writes, they swap in the next cycle
    std::mutex Deffered_call_maps_mutex;
    bool deffered_call_cycle = false;
    /**
     * @brief Contains two @ref Deffered_Call_Map "Deffered_Call_Maps" used to store @ref Deffered_Call "deffered calls", for explanation why two are needed see the detials of @ref ScriptSystemDefferedCall
    */
    std::vector<Deffered_Call_Map> m_Deffered_call_maps;
    /**
     * @brief Contains two vectors of entities used to store all entities on which deffered calls should be executed, for explanation why two are needed see the detials of @ref ScriptSystemDefferedCall
    */
    std::vector<std::vector<Entity>> m_Pending_Deffered_call_vectors;

    std::vector<Entity> collided_entities; ///< Vector containing all entities, which recieved collision events
    std::unordered_map<uint32_t, std::vector<CollisionEvent_L>> entity_collisions; ///< Map associating all entities in @ref collided_entities with all their collision events
    std::unique_ptr<EventObserverBase> collision_observer; ///< an EventObserver used to capture collision events generated by the PhysicsEngine 

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

    void RunGarbageCollector();

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
    auto CallFunctionRuntime(const std::string& path, const std::string& function_name, const std::vector<std::variant<Args...>>& args)
        -> std::enable_if_t<(!std::is_void_v<R>), R>
    {
        R out;
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObjectRuntime<void>(nullptr, LuaEngineUtilities::ScriptHash(path), function_name, args);
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
            return CallFunctionRuntime<R>(path, function_name, args);
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

    template<typename R, typename ... Args>
    auto TryCallFunctionRuntime(R* out, const std::string& path, const std::string& function_name, const std::vector<std::variant<Args...>>& args)
        -> std::enable_if_t<(!std::is_void_v<R>), bool>
    {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObjectRuntime<void>(out, LuaEngineUtilities::ScriptHash(path), function_name, args);
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
            return TryCallFunctionRuntime(out, path, function_name, args);
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
    void CallFunctionRuntime(const std::string& path, const std::string& function_name, const std::vector<std::variant<Args...>>& args) {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success;
            {
                success = m_LuaEngine.TryCallObjectRuntime<void>(nullptr, LuaEngineUtilities::ScriptHash(path).c_str(), function_name.c_str(), args);
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
            CallFunctionRuntime(path, function_name, args);
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

    template<typename ... Args>
    bool TryCallFunctionRuntime(void* null, const std::string& path, const std::string& function_name, const std::vector<std::variant<Args...>>& args) {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success;
            {
                success = m_LuaEngine.TryCallObjectRuntime<void>(nullptr, LuaEngineUtilities::ScriptHash(path).c_str(), function_name.c_str(), args);
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
            return TryCallFunctionRuntime(nullptr, path, function_name, args);
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

