#pragma once
#include <World/Components/ScriptComponent.h>
#include <unordered_map>
#include <World/System.h>
#include <World/Entity.h>
#include <unordered_set>
#include <LuaEngine.h>

std::string ScriptHash(std::string script_path);

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
    std::string& LoadScript(const std::string& path);
    std::string ParseScript(std::string script, const std::string& hash);

    ScriptSystemVM* TryGetScriptSystemVM();

    void InitializeScriptSystemVM();

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

    //This is where Functions which are bound to Lua go
#pragma region LuaBound
    void TestChangeSquarePos(float x, float y);




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
        if (m_BoundScripts.find(ScriptHash(path)) != m_BoundScripts.end()) {
            bool success = m_LuaEngine.TryCallObject<void>(nullptr, ScriptHash(path), function_name, args...);
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
            m_BoundScripts.insert(ScriptHash(path));
        }
    }

    template<typename ... Args>
    void CallFunction(const std::string& path, const std::string& function_name, Args ... args) {
        if (m_BoundScripts.find(ScriptHash(path)) != m_BoundScripts.end()) {
            bool success;
            {
                success = m_LuaEngine.TryCallObject<void>(nullptr,ScriptHash(path).c_str(), function_name.c_str(), args...);
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
            m_BoundScripts.insert(ScriptHash(path));
        }
    }


private:
    ScriptHandler curentHandler;
    std::unordered_set<std::string> m_BoundScripts;
    LuaEngineClass<ScriptHandler> m_LuaEngine;
};

inline void ScriptSystem(World& world, float delta_time) {
    auto func_1 = [&delta_time](ComponentCollection compcol, system_view_type<ScriptComponent>& comps, entt::registry* reg) {
        PROFILE("ScriptRunThread");
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }

        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size;iter++ ) {
            auto comp = reg->get<ScriptComponent>(*iter);
            script_vm->SetEngineEntity(Entity((uint32_t)*iter));
            script_vm->CallFunction(comp.script_path, "OnUpdate",delta_time);

        };

    };

    RunSystemSimple<ScriptComponent>(world, func_1);
}