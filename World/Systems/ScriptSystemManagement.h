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
     * @warning access to the Deffered_Call_Map is not thread-safe 
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
     * @warning This method is not thread safe 
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

/**
 * @brief An object which takes care of binding Engine functionality into the lua @ref ScriptVM "ScriptVMs" for @ref inline_script "inline scripts", and also keeps
 * track of the entity currently being processed by the @ref inline_script "inline script"
 * 
 * This class contains methods used for initialization of @ref LuaEngineClass "LuaEngines" taking care of @ref inline_script "inline script" execution.
 * It binds C++ side Engine functionality to allow scripts to call Engine specific functions. It also provides the GetEntity function which is heavily used
 * in all lua functions which manipulate the current entity being processed.
 * 
 * @note This object is only used in @ref inline_script "inline scripts"
*/
class ScriptHandler {
public:
    /**
     * @brief Creates a new ScriptHandler bound with an Entity which will be used to execute all operations operating on the current entity in @ref inline_script "inline scripts"
     * @param ent Entity to be bound as the new current Entity
    */
    ScriptHandler(Entity ent) : current_entity(ent) {}
    
    /**
     * @brief Binds all Engine side C++ functionality to lua functions used in the @ref inline_script "inline script"
     * @param script_engine The @ref LuaEngineClass "LuaEngine" to bind the functionality to.  
    */
    static void BindHandlerFunctions(LuaEngineClass<ScriptHandler>* script_engine);
    /**
     * @brief Defines ScriptKeyBindings enum structure in the lua engine,
     * @param script_engine The @ref LuaEngineClass "LuaEngine" to define the ScriptKeyBindings enum in.  
    */
    static void BindKeyCodes(LuaEngineClass<ScriptHandler>* script_engine);
    
    //This is where Functions which are bound to Lua go
#pragma region LuaBound
    /**
     * @brief Test function no longer used
     * @deprecated This function will be removed
    */
    glm::vec2 TestGetPosition();

    /**
     * @brief Gets the property from the DynamicPropertiesComponent of the current Entity.
     * @tparam T Type of the property to return 
     * @param name name of the property to return 
     * @return The property of the current Entity
     * 
     * @deprecated This actual method used is defined in the @ref LocalPropertySetModule.cpp file 
    */
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

    /**
     * @brief Sets the property in the DynamicPropertiesComponent of the current Entity.
     * @tparam T Type of the property to set
     * @param name name of the property to set
     * @param value the value to set the property to 
     * 
     * @deprecated This actual method used is defined in the @ref LocalPropertySetModule.cpp file 
    */
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


    /**
     * @brief Cheks if the property exists in the DynamicPropertiesComponent of the current Entity.
     * @param name name of the property to check
     * @param value true if it exists, false if not
     *
     * @deprecated This actual method used is defined in the @ref LocalPropertySetModule.cpp file
    */
    bool PropertyExists(std::string name);

    /**
     * @brief Gets the currently bound Entity which is currently being processed
     * @return the Entity currently being processed.
    */
    Entity GetEntity() const {
        return current_entity;
    }


#pragma endregion
private:
    Entity current_entity; ///< the Entity currently being processed.
};

#pragma endregion

#pragma region InitializationScriptHandler


/**
 * @brief An object which takes care of binding Engine functionality into the lua @ref ScriptVM "ScriptVMs" for @ref construction_script "construction script", and also keeps
 * track of the entity currently being processed by the @ref construction_script "construction script"
 *
 * This class contains methods used for initialization of @ref LuaEngineClass "LuaEngines" taking care of @ref construction_script "construction script" execution.
 * It binds C++ side Engine functionality to allow scripts to call Engine specific functions. It also provides the GetEntity function which is heavily used
 * in all lua functions which manipulate the current entity being processed.
 * 
 * @note This object is only used in @ref construction_script "construction scripts"
 * 
 * @todo Most of this is deprecated and needs to be revised or just removed. Many necessary modules are missing and methods for adding components are missing as well.
 * Most functions also target current entity, but if the Prefab root is the only entity which can have a Script component, then nothing but the Prefab root can be influenced.
*/
class InitializationScriptHandler {
public:
    /**
     * @brief Creates a new InitializationScriptHandler bound with an Entity which will be used to execute all operations operating on the current entity in @ref construction_script "construction scripts"
     * @param ent Entity to be bound as the new current Entity
     * @param path Path of the Entity file from which the entity is being constructed (can contain file subsections)
    */
    InitializationScriptHandler(Entity ent, const std::string& path) : current_entity(ent), current_path(path) {}

    /**
     * @brief Binds all Engine side C++ functionality to lua functions used in the @ref construction_script "construction script"
     * @param script_engine The @ref LuaEngineClass "LuaEngine" to bind the functionality to.
    */
    static void BindHandlerFunctions(LuaEngineClass<InitializationScriptHandler>* script_engine);
    //This is where Functions which are bound to Lua go
#pragma region LuaBound
    
    /**
     * @brief Template for setting the components of the current entity
     * @tparam T Type of the Component to set
     * @tparam ...Args Types os parameters to pass to the component constructor
     * @param ...args values of arguments passed to the component constructor
    */
    template<typename T, typename ... Args>
    void SetComponent(Args&&... args) {
        Application::GetWorld().SetComponent<T>(current_entity, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Function bound to Lua which overrides the inline script path. 
     * @param path a file containing the new @ref inline_script "inline script" to be bound (can contain file subsections).
     * @note This function should only be used rarely
     * @note This function is only available in the @ref construction_script "construction script"
    */
    void SetScriptComponent(std::string path);

    /**
     * @brief Old function which adds a SquareComponent to the current entity
     * @param color color of the SquareComponent
     * @deprecated Will be removed
    */
    void SetSquareComponent(glm::vec4 color = glm::vec4(1.0f));

    /**
     * @brief Adds a CameraComponent to the current Entity
     * @param fov The Field of View of the CameraComponent
     * @param zNear The near clipping plane distance of the CameraComponent
     * @param zFar The far clipping plane distance of the CameraComponent
     * @param aspect_ratio The aspact ratio of the camera projection of the CameraComponent
     * 
     * @note This function is only available in the @ref construction_script "construction script"
    */
    void SetCameraComponent(float fov, float zNear, float zFar, float aspect_ratio);

    /**
     * @brief Sets the translation of the current Entity
     * @param translation the new local translation of the current Entity
    */
    void SetTranslation(glm::vec3 translation);

    /**
     * @brief Sets the scale of the current Entity
     * @param translation the new local scale of the current Entity
    */
    void SetScale(glm::vec3 scale);

    /**
     * @brief Enables the reception of @ref KeyPressEvent "KeyPressEvents"
     * 
     * @warning The OnKeyPressed needs to be defined in the @ref inline_script "inline script"
    */
    void EnableKeyPressedEvents();

    /**
     * @brief Enables the reception of @ref MouseButtonPressEvent "MouseButtonPressEvents"
     * 
     * @warning The OnMouseButtonPressed needs to be defined in the @ref inline_script "inline script"
    */
    void EnableMouseButtonPressedEvents();

    /**
     * @brief Gets the currently bound Entity which is currently being processed
     * @return the Entity currently being processed.
    */
    Entity GetEntity() const {
        return current_entity;
    }

#pragma endregion
private:
    std::string current_path; ///< Path of the Entity file from which the entity is being constructed (can contain file subsections)
    Entity current_entity; ///< the Entity currently being processed.
};

#pragma endregion

/**
 * @brief Encapsulates two different @ref LuaEngineClass "LuaEngines"(One for @ref inline_script "inline scripts" and one for @ref construction_script "construction scripts") and 
 * takes care of lazy loading lua scripts into them from the cache in the ScriptSystemManager. One exists for each thread since lua does not support multithreading.
 * 
 * ScriptSystemVM can be either in  @ref construction_script "construction scripts" mode or @ref inline_script "inline scripts" mode. Each mode maintains its own LuaEngine and current Entity.
 * All member functions which call lua functions also have two versions which use the two @ref LuaEngineClass "LuaEngines". To set the modes current 
 * Entity use ScriptSystemVM::SetEngineEntity(for @ref inline_script "inline scripts") and ScriptSystemVM::SetEngineInitializationEntity(for @ref construction_script "construction scripts")
*/
class ScriptSystemVM {
public:
    friend ScriptSystemManager;
private:
    ScriptSystemVM();
    ~ScriptSystemVM();
public:
    /**
     * @brief Sets the ScriptSystemVM to @ref inline_script "inline script" mode and sets its new Entity to process 
     * @param ent an Entity that will be used as the new currently processed Entity
     * 
     * All Engine functions called from the @ref inline_script "inline script", which operate on the current Entity will operate on the Entity set in this call
    */
    void SetEngineEntity(Entity ent);

    /**
     * @brief Gets the current Entity for either @ref inline_script "inline scripts" or @ref inline_script "inline scripts" depending on the mode the ScriptSystemVM is in.
     * @return the current Entity 
    */
    Entity GetCurrentEntity() const {
        if (init_mode) {
            return current_Initialization_handler.GetEntity();
        }
        else {
            return curentHandler.GetEntity();
        }
    }

    /**
     * @brief Gets the current Entity for @ref inline_script "inline scripts".
     * @return the current Entity for @ref inline_script "inline scripts".
    */
    Entity GetEngineEntity() const {
        return curentHandler.GetEntity();
    }

    /**
     * @brief Runs the Lua Garbage collector
    */
    void RunGarbageCollector();

    // For Script Runtime

    /**
     * @brief Calls a lua @ref inline_script "inline script" function.
     * @tparam R The non void return type of the script
     * @tparam ...Args The lua function parameter types
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * @return The return value of the lua function
     * 
     * @note This function uses template parameter pack for the arguments. Each function using a different number and combination of parameter types is a completely different function
     * from the compilers perspective, and as such the types of parameters passed can not change on runtime. 
     * If this functionality is required use the versions of Call functions containg the word Runtime
    */
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
    /**
     * @brief Calls a lua @ref inline_script "inline script" function. This version allows for dynamic parameters list, which can change at runtime, instead of relying on templates.
     * @tparam R The non void return type of the script
     * @tparam ...Args The lua function parameter variant types
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param args the vector of values of the function arguments to call the function with. This vector can contain a varying number of different types at runtime.
     * @return The return value of the lua function
     * 
     * @warning All types passed must still be of the same variant type
    */
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

    /**
     * @brief Calls a lua @ref inline_script "inline script" function. This function doesn't throw on error, but returns the success state as a boolean.
     * @tparam R The non void return type of the script, if this is void The return value gets ignored
     * @tparam ...Args The lua function parameter types
     * @param out the pointer to which the return value of the lua function will be written on success, if R is not void, this cannot be nullprt
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * @return The success value of the lua call.
     *
     * @note This function uses template parameter pack for the arguments. Each function using a different number and combination of parameter types is a completely different function
     * from the compilers perspective, and as such the types of parameters passed can not change on runtime.
     * If this functionality is required use the versions of Call functions containg the word Runtime
     * 
     * @warning If R is not void out needs to be valid.
    */
    template<typename R, typename ... Args>
    auto TryCallFunction(R* out,const std::string& path, const std::string& function_name, Args ... args)
        -> std::enable_if_t<(!std::is_void_v<R>), bool>
    {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObject<R>(out, LuaEngineUtilities::ScriptHash(path), function_name, args...);
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

    /**
     * @brief Calls a lua @ref inline_script "inline script" function. This function doesn't throw on error, but returns the success state as a boolean. This version allows for dynamic parameters list, which can change at runtime, instead of relying on templates.
     * @tparam R The non void return type of the script, if this is void The return value gets ignored
     * @tparam ...Args The lua function parameter variant types
     * @param out the pointer to which the return value of the lua function will be written on success, if R is not void, this cannot be nullprt
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param args the vector of values of the function arguments to call the function with. This vector can contain a varying number of different types at runtime.
     * @return The success value of the lua call.
     * 
     * @warning If R is not void out needs to be valid.
     * @warning All types passed must still be of the same variant type
    */
    template<typename R, typename ... Args>
    auto TryCallFunctionRuntime(R* out, const std::string& path, const std::string& function_name, const std::vector<std::variant<Args...>>& args)
        -> std::enable_if_t<(!std::is_void_v<R>), bool>
    {
        if (m_BoundScripts.find(LuaEngineUtilities::ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObjectRuntime<R>(out, LuaEngineUtilities::ScriptHash(path), function_name, args);
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

    /**
     * @brief Calls a lua @ref inline_script "inline script" function. Runs without return value.
     * @tparam ...Args The lua function parameter types
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * 
     * @note This function uses template parameter pack for the arguments. Each function using a different number and combination of parameter types is a completely different function
     * from the compilers perspective, and as such the types of parameters passed can not change on runtime. 
     * If this functionality is required use the versions of Call functions containg the word Runtime.
    */
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


    /**
     * @brief Calls a lua @ref inline_script "inline script" function. This version allows for dynamic parameters list, which can change at runtime, instead of relying on templates. Runs without return value.
     * @tparam ...Args The lua function parameter variant types
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param args the vector of values of the function arguments to call the function with. This vector can contain a varying number of different types at runtime.
     *
     * @warning All types passed must still be of the same variant type
    */
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

    /**
     * @brief Calls a lua @ref inline_script "inline script" function. This function doesn't throw on error, but returns the success state as a boolean. Runs without script return value.
     * @tparam ...Args The lua function parameter types
     * @param null this parameter is irrelevant and is not used
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * @return The success value of the lua call.
     *
     * @note This function uses template parameter pack for the arguments. Each function using a different number and combination of parameter types is a completely different function
     * from the compilers perspective, and as such the types of parameters passed can not change on runtime.
     * If this functionality is required use the versions of Call functions containg the word Runtime
    */
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


    /**
     * @brief Calls a lua @ref inline_script "inline script" function. This function doesn't throw on error, but returns the success state as a boolean. This version allows for dynamic parameters list, 
     * which can change at runtime, instead of relying on templates. Runs without script return value.
     * @tparam ...Args The lua function parameter variant types
     * @param null this parameter is irrelevant and is not used
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param args the vector of values of the function arguments to call the function with. This vector can contain a varying number of different types at runtime.
     * @return The success value of the lua call.
     * 
     * @warning All types passed must still be of the same variant type
    */
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

    /**
     * @brief Sets the ScriptSystemVM to @ref construction_script "construction script" mode and sets its new Entity to process
     * @param ent an Entity that will be used as the new currently processed Entity
     *
     * All Engine functions called from the @ref construction_script "construction script", which operate on the current Entity will operate on the Entity set in this call
    */
    void SetEngineInitializationEntity(Entity ent, const std::string& path);

    /**
     * @brief Gets the current Entity for @ref construction_script "construction scripts".
     * @return the current Entity for @ref construction_script "construction scripts".
    */
    Entity GetEngineInitializationEntity() const {
        return current_Initialization_handler.GetEntity();
    };

    /**
     * @brief Calls a lua @ref construction_script "construction script" function.
     * @tparam R The non void return type of the script
     * @tparam ...Args The lua function parameter types
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * @return The return value of the lua function
    */
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

    /**
     * @brief Calls a lua @ref construction_script "construction script" function. This function doesn't throw on error, but returns the success state as a boolean.
     * @tparam R The non void return type of the script, if this is void The return value gets ignored
     * @tparam ...Args The lua function parameter types
     * @param out the pointer to which the return value of the lua function will be written on success, if R is not void, this cannot be nullprt
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * @return The success value of the lua call.
     * 
     * @warning If R is not void out needs to be valid.
    */
    template<typename R, typename ... Args>
    auto TryCallInitializationFunction(R* out, const std::string& path, const std::string& function_name, Args ... args)
        -> std::enable_if_t<(!std::is_void_v<R>), bool>
    {
        if (m_BoundInitializationScripts.find(LuaEngineUtilities::ScriptHash(path, true)) != m_BoundInitializationScripts.end()) {
            bool success = m_LuaInitializationEngine.TryCallObject<R>(out, LuaEngineUtilities::ScriptHash(path, true), function_name, args...);
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

    /**
     * @brief Calls a lua @ref construction_script "construction script" function. Runs without return value.
     * @tparam ...Args The lua function parameter types
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
    */
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

    /**
     * @brief Calls a lua @ref construction_script "construction script" function. This function doesn't throw on error, but returns the success state as a boolean. Runs without script return value.
     * @tparam ...Args The lua function parameter types
     * @param null this parameter is irrelevant and is not used
     * @param path The path to the Entity file which contains the script
     * @param function_name The name of the function in the file
     * @param ...args the values of the function arguments to call the function with
     * @return The success value of the lua call.
    */
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

    /**
     * @brief internal function: Forces an @ref inline_script "inline script" at script_path to be reloaded from @ref ScriptSystemManager cache.
     * @param script_path the path to the @ref inline_script "inline script" to invalidate
     *
     * @warning this does not invalidate ScriptSystemManager cache, only the @ref inline_script "inline script" LuaEngine definitions. This is mostly for internal use.
     * use ScriptSystemManager::InvalidateInlineScript instead.
    */
    void InvalidateInlineScript(const std::string& script_path);

    /**
     * @brief internal function: Forces an @ref construction_script "construction script" at script_path to be reloaded from @ref ScriptSystemManager cache.
     * @param script_path the path to the @ref construction_script "construction script" to invalidate
     *
     * @warning this does not invalidate ScriptSystemManager cache, only the @ref construction_script "construction script" LuaEngine definitions. This is mostly for internal use.
     * use ScriptSystemManager::InvalidateConstructionScript instead.
    */
    void InvalidateConstructionScript(const std::string& script_path);

    /**
     * @brief Resets all both LuaEngines to default.
    */
    void ResetScriptVM();


private:
    ScriptHandler curentHandler; ///< The ScriptHandler responsible for handling the storage of @ref inline_script "inline script" current Entity and @ref inline_script "inline script" function binding
    bool init_mode = false; ///< keeps track of the mode the ScriptSystemVM is in (@ref inline_script "inline script" or @ref construction_script "construction script")

    std::unordered_set<std::string> m_BoundScripts; ///< All script sources which have been loaded into the @ref inline_script "inline script" @ref LuaEngineClass "LuaEngine"
    LuaEngineClass<ScriptHandler> m_LuaEngine; ///< the @ref inline_script "inline script" @ref LuaEngineClass "LuaEngine"

    InitializationScriptHandler current_Initialization_handler; ///< The ScriptHandler responsible for handling the storage of @ref construction_script "construction script" current Entity and @ref construction_script "construction script" function binding

    std::unordered_set<std::string> m_BoundInitializationScripts; ///< All script sources which have been loaded into the @ref construction_script "construction script" @ref LuaEngineClass "LuaEngine"
    LuaEngineClass<InitializationScriptHandler> m_LuaInitializationEngine; ///< the @ref construction_script "construction script" @ref LuaEngineClass "LuaEngine"
};

