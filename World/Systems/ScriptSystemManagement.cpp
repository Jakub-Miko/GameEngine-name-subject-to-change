#include "ScriptSystemManagement.h"
#include <algorithm>
#include <Input/Input.h>
#include <Events/KeyCodes.h>
#include <FileManager.h>
#include <string>
#include <World/Components/SquareComponent.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/DefferedUpdateComponent.h>
#include <World/EntityManager.h>
#include <sstream>
#include <World/World.h>
#include <Application.h>
#include <fstream>
#include <ThreadManager.h>
#include <World/ScriptModules/IOModule.h>
#include <World/ScriptModules/DefferedPropertySetModule.h>
#include <World/ScriptModules/ApplicationDataModule.h>
#include <World/ScriptModules/LocalPropertySetModule.h>
#include <World/ScriptModules/TimeModule.h>
#include <World/ScriptModules/LocalEntityModule.h>
#include <stdexcept>

ScriptSystemManager* ScriptSystemManager::instance = nullptr;

void ScriptSystemManager::Initialize()
{
    if (!instance) {
        instance = new ScriptSystemManager;
    }
}

ScriptSystemManager* ScriptSystemManager::Get()
{
    return instance;
}

void ScriptSystemManager::Shutdown()
{
    if (instance) {
        delete instance;
    }
}

void ScriptSystemManager::InitThread()
{
    ThreadManager::Get()->SetThreadLocalData<Deffered_Set_Map>(GetDefferedSetMap());
}

std::string& ScriptSystemManager::GetScript(const std::string& path)
{
    std::unique_lock<std::mutex> lock(script_cache_mutex);
    auto file = m_ScriptCache.find(LuaEngineUtilities::ScriptHash(path));
    if (file != m_ScriptCache.end()) {
        return (*file).second.script;
    }
    else {
        std::string str = LuaEngineUtilities::LoadScript(path);
        auto it = m_ScriptCache.insert_or_assign(LuaEngineUtilities::ScriptHash(path),ScriptObject(str)).first;
        return (*it).second.script;
    }
}

std::string& ScriptSystemManager::GetConstructionScript(const std::string& path)
{
    std::unique_lock<std::mutex> lock(script_cache_mutex);
    auto file = m_ScriptCache.find(LuaEngineUtilities::ScriptHash(path, true));
    if (file != m_ScriptCache.end()) {
        return (*file).second.script;
    }
    else {
        std::string str = LuaEngineUtilities::LoadScript(path, true);
        auto it = m_ScriptCache.insert_or_assign(LuaEngineUtilities::ScriptHash(path, true), ScriptObject(str)).first;
        return (*it).second.script;
    }
}

void ScriptSystemManager::UploadScript(const std::string& path, const std::string& script)
{
    auto hash = LuaEngineUtilities::ScriptHash(path);
    std::string parsed_script = LuaEngineUtilities::ParseScript(script, hash);
    std::unique_lock<std::mutex> lock(script_cache_mutex);
    m_ScriptCache.insert_or_assign(hash, ScriptObject(parsed_script));
}

void ScriptSystemManager::UploadConstructionScript(const std::string& path, const std::string& script)
{
    auto hash = LuaEngineUtilities::ScriptHash(path,true);
    std::string parsed_script = LuaEngineUtilities::ParseScript(script, hash,true);
    std::unique_lock<std::mutex> lock(script_cache_mutex);
    m_ScriptCache.insert_or_assign(hash, ScriptObject(parsed_script));
}

void ScriptSystemManager::SetEntityAsDirty(Entity ent)
{
    Application::GetWorld().SetComponent<DefferedUpdateComponent>(ent);
}
 
const std::vector<Deffered_Set_Map>& ScriptSystemManager::GetEntityChanges()
{
    std::lock_guard<std::mutex> lock(DefferedSetMaps_mutex);
    return m_DefferedSetMaps;
} 

void ScriptSystemManager::ClearEntityChanges()
{
    for (auto& map : m_DefferedSetMaps) {
        map.clear();
    }
}

ScriptSystemVM* ScriptSystemManager::TryGetScriptSystemVM()
{
    if (ThreadManager::IsValidThreadContext()) {
        if (ThreadManager::ThreadLocalDataExists<ScriptSystemVM>()) {
            return ThreadManager::GetThreadLocalData<ScriptSystemVM>();
        }
    }
    return nullptr;
}

void ScriptSystemManager::InitializeScriptSystemVM()
{
    if (ThreadManager::IsValidThreadContext()) {
        auto vm = new ScriptSystemVM;
        std::lock_guard<std::mutex> lock(sync_mutex);
        m_Script_system_VMs.push_back(vm);
        ThreadManager::SetThreadLocalData<ScriptSystemVM>(vm);
    }
    else {
        throw std::runtime_error("ThreadState is Unavailable in this Thread");
    }
}

void ScriptSystemManager::ResetAllScriptSystemVMs()
{
    for (auto vm : m_Script_system_VMs) {
        vm->ResetScriptVM();
    }
}

ScriptSystemManager::~ScriptSystemManager()
{
    for (auto vm : m_Script_system_VMs) {
        delete vm;
    }
}

ScriptSystemManager::ScriptSystemManager() : sync_mutex(), m_DefferedSetMaps(), DefferedSetMaps_mutex(), m_ScriptCache()
{
    m_DefferedSetMaps.reserve(ThreadManager::Get()->GetMaxThreadCount());
}

Deffered_Set_Map* ScriptSystemManager::GetDefferedSetMap()
{
    std::lock_guard<std::mutex> lock(DefferedSetMaps_mutex);
    if ((m_DefferedSetMaps.size() + 1) > m_DefferedSetMaps.capacity()) {
        throw std::runtime_error("Invalid number of deffred set maps allocated");
    }
    return &(m_DefferedSetMaps.emplace_back());
}

ScriptSystemVM::ScriptSystemVM() : m_LuaEngine(), m_LuaInitializationEngine(), 
    curentHandler(Entity()), current_Initialization_handler(Entity(),""), m_BoundScripts(), m_BoundInitializationScripts()
{
    m_BoundScripts.reserve(10);
    m_BoundInitializationScripts.reserve(10);
    ScriptHandler::BindKeyCodes(&m_LuaEngine);
    ScriptHandler::BindHandlerFunctions(&m_LuaEngine);
    InitializationScriptHandler::BindHandlerFunctions(&m_LuaInitializationEngine);
}

ScriptSystemVM::~ScriptSystemVM()
{
}

void ScriptSystemVM::SetEngineEntity(Entity ent)
{
    curentHandler = ScriptHandler(ent);
    m_LuaEngine.SetClassInstance(&curentHandler);
    init_mode = false;
}

void ScriptSystemVM::SetEngineInitializationEntity(Entity ent, const std::string& path)
{
    current_Initialization_handler = InitializationScriptHandler(ent, path);
    m_LuaInitializationEngine.SetClassInstance(&current_Initialization_handler);
    init_mode = true;
}

void ScriptSystemVM::ResetScriptVM()
{
    m_LuaEngine = LuaEngineClass<ScriptHandler>();
    m_BoundScripts.clear();
    curentHandler = ScriptHandler(Entity());
    ScriptHandler::BindKeyCodes(&m_LuaEngine);
    ScriptHandler::BindHandlerFunctions(&m_LuaEngine);

    m_LuaInitializationEngine = LuaEngineClass<InitializationScriptHandler>();
    m_BoundInitializationScripts.clear();
    current_Initialization_handler = InitializationScriptHandler(Entity(),"");
    InitializationScriptHandler::BindHandlerFunctions(&m_LuaInitializationEngine);
}

void ScriptHandler::BindHandlerFunctions(LuaEngineClass<ScriptHandler>* script_engine)
{
    ModuleBindingProperties props;
    script_engine->InitFFI();
    props.Add_bindings( {
        //This is where function bindings go
        //TODO: implement Script modules for code reuse.
        {"GetPos" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::TestGetPosition>},                                        // LocalProperty Module - use adapter
        
        
        {"EnableKeyPressedEvents", LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::EnableKeyPressedEvents>},                 // Entity ConfigModule - use adapter
        {"EnableMouseButtonPressedEvents", LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::EnableMouseButtonPressedEvents>}  // Entity ConfigModule - use adapter

    });



    IOModule().RegisterModule(props);
    TimeModule().RegisterModule(props);
    DefferedPropertySetModule().RegisterModule(props);
    ApplicationDataModule().RegisterModule(props);
    LocalEntityModule().RegisterModule(props);
    LocalPropertySetModule().RegisterModule(props);

    script_engine->RegisterModule(props);
}

void InitializationScriptHandler::BindHandlerFunctions(LuaEngineClass<InitializationScriptHandler>* script_engine)
{
    ModuleBindingProperties props;
    script_engine->InitFFI();

    props.Add_bindings({
        //This is where function bindings go
        {"SetSquareComponent", LuaEngineClass<InitializationScriptHandler>::InvokeClass<&InitializationScriptHandler::SetSquareComponent>},         //SetComponent Module - use adapter
        {"SetScriptComponent", LuaEngineClass<InitializationScriptHandler>::InvokeClass<&InitializationScriptHandler::SetScriptComponent>},         //SetComponent Module - use adapter
        {"SetCameraComponent", LuaEngineClass<InitializationScriptHandler>::InvokeClass<&InitializationScriptHandler::SetCameraComponent>},         //SetComponent Module - use adapter
        {"SetTranslation", LuaEngineClass<InitializationScriptHandler>::InvokeClass<&InitializationScriptHandler::SetTranslation>},                 //Transform Module - use adapter
        {"SetScale", LuaEngineClass<InitializationScriptHandler>::InvokeClass<&InitializationScriptHandler::SetScale>},                             //Transform Module - use adapter
        {"UseInlineScript", LuaEngineClass<InitializationScriptHandler>::InvokeClass<&InitializationScriptHandler::UseInlineScript>},               //InitialConfig Module - use adapter
    });

    IOModule().RegisterModule(props);
    TimeModule().RegisterModule(props);
    ApplicationDataModule().RegisterModule(props);


    script_engine->RegisterModule(props);
}

void InitializationScriptHandler::SetScriptComponent(std::string path)
{
    Application::GetWorld().SetComponent<ScriptComponent>(current_entity, ScriptComponent(path));
}

void InitializationScriptHandler::SetSquareComponent(glm::vec4 color)
{
    Application::GetWorld().SetComponent<SquareComponent>(current_entity, SquareComponent(color));
}

void InitializationScriptHandler::SetCameraComponent(float fov, float zNear, float zFar, float aspect_ratio)
{
    Application::GetWorld().SetComponent<CameraComponent>(current_entity, CameraComponent(fov,zNear, zFar, aspect_ratio));
}

void InitializationScriptHandler::SetTranslation(glm::vec3 translation)
{
    Application::Get()->GetWorld().SetEntityTranslationSync(current_entity, translation);
}

void InitializationScriptHandler::SetScale(glm::vec3 scale)
{
    Application::Get()->GetWorld().SetEntityScaleSync(current_entity, scale);
}

void InitializationScriptHandler::EnableKeyPressedEvents()
{
    Application::GetWorld().SetComponent<KeyPressedScriptComponent>(current_entity);
}

void InitializationScriptHandler::UseInlineScript()
{
    Application::GetWorld().SetComponent<ScriptComponent>(current_entity, ScriptComponent(current_path));
}

void InitializationScriptHandler::EnableMouseButtonPressedEvents()
{
    Application::GetWorld().SetComponent<MousePressedScriptComponent>(current_entity);
}

void ScriptHandler::BindKeyCodes(LuaEngineClass<ScriptHandler>* script_engine)
{
    script_engine->RunString(ScriptKeyBindings);
}

glm::vec2 ScriptHandler::TestGetPosition()
{
    return Application::GetWorld().GetComponent<TransformComponent>(current_entity).translation;
}

bool ScriptHandler::PropertyExists(std::string name)
{
    auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(current_entity).m_Properties;
    auto find = props.find(name);
    if (find != props.end()) {
        return true;
    };
    return false;
}

void ScriptHandler::EnableKeyPressedEvents()
{
    Application::GetWorld().SetComponent<KeyPressedScriptComponent>(current_entity);
}

void ScriptHandler::EnableMouseButtonPressedEvents()
{
    Application::GetWorld().SetComponent<MousePressedScriptComponent>(current_entity);
}


