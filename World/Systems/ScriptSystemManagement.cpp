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
#include <Events/CollisionEvent.h>
#include <World/ScriptModules/IOModule.h>
#include <World/ScriptModules/DefferedPropertySetModule.h>
#include <World/ScriptModules/ApplicationDataModule.h>
#include <World/ScriptModules/CollisionModule.h>
#include <World/ScriptModules/LocalPropertySetModule.h>
#include <World/ScriptModules/TimeModule.h>
#include <World/ScriptModules/RayCastingModule.h>
#include <World/ScriptModules/PrefabManipulationModule.h>
#include <World/ScriptModules/LocalEntityModule.h>
#include <World/ScriptModules/EventModule.h>
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
        std::string str = FileManager::Get()->OpenFile(path);
        str = LuaEngineUtilities::ParseScript(str, LuaEngineUtilities::ScriptHash(path, false));
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
        
        std::string str = FileManager::Get()->OpenFile(path);
        str = LuaEngineUtilities::ParseScript(str, LuaEngineUtilities::ScriptHash(path, true),true);
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

Deffered_Call_Map& ScriptSystemManager::GetDefferedCalls()
{
    return m_Deffered_call_maps[deffered_call_cycle ? 1 : 0];
}

std::vector<Deffered_Call>& ScriptSystemManager::GetDefferedCallsForEntity(Entity ent)
{
    auto fnd = GetDefferedCalls().find(ent.id);
    if (fnd != GetDefferedCalls().end()) {
        return fnd->second;
    }
    else {
        throw std::runtime_error("This entity should't be called");
    }
}

std::vector<CollisionEvent_L>& ScriptSystemManager::GetEntityCollisions(Entity ent)
{
    auto fnd = entity_collisions.find(ent.id);
    if (fnd != entity_collisions.end()) {
        return fnd->second;
    }
    else {
        throw std::runtime_error("This entity doesn't have a collision registered");
    }
}

std::vector<Entity>& ScriptSystemManager::GetPendingDefferedCallEntities()
{
    return m_Pending_Deffered_call_vectors[deffered_call_cycle ? 1 : 0];
}

void ScriptSystemManager::AddDefferedCall(Entity ent, const Deffered_Call& call_info)
{
    if (!Application::GetWorld().HasComponentSynced<ScriptComponent>(ent)) {
        SceneNode* node = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(ent);
        if (node && (bool)(node->state & SceneNodeState::PREFAB_CHILD)) {
            while (!(bool)(node->state & SceneNodeState::PREFAB))
            {
                if (!node) return;
                node = node->parent;
            }
            ent = node->entity;
            if (!Application::GetWorld().HasComponentSynced<ScriptComponent>(ent)) return;
        }
        else {
            return;
        }
    }


    std::lock_guard<std::mutex> lock(Deffered_call_maps_mutex);
    auto& call_map = m_Deffered_call_maps[deffered_call_cycle ? 0 : 1];
    auto& call_entities = m_Pending_Deffered_call_vectors[deffered_call_cycle ? 0 : 1];
    auto fnd = call_map.find(ent.id);
    if (fnd != call_map.end()) {
        fnd->second.push_back(call_info);
    }
    else {
        call_entities.push_back(ent);
        auto new_entry = call_map.insert(std::make_pair(ent.id, std::vector<Deffered_Call>()));
        new_entry.first->second.push_back(call_info);
    }
}
 
void ScriptSystemManager::SwapDefferedCallCycle()
{
    deffered_call_cycle = !deffered_call_cycle;
}

void ScriptSystemManager::InvalidateInlineScript(const std::string& script_path)
{
    auto hash = LuaEngineUtilities::ScriptHash(script_path, false);
    auto fnd = m_ScriptCache.find(hash);
    if (fnd != m_ScriptCache.end()) {
        m_ScriptCache.erase(hash);
    }
    auto& threads = ThreadManager::Get()->GetAllThreadObjects();
    for (auto& thread : threads) {
        if (thread->StateValueExists<ScriptSystemVM>()) {
            auto vm = thread->GetStateValue<ScriptSystemVM>();
            vm->InvalidateInlineScript(script_path);
        }
    }
}

void ScriptSystemManager::InvalidateConstructionScript(const std::string& script_path)
{
    auto hash = LuaEngineUtilities::ScriptHash(script_path, true);
    auto fnd = m_ScriptCache.find(hash);
    if (fnd != m_ScriptCache.end()) {
        m_ScriptCache.erase(hash);
    }
    auto& threads = ThreadManager::Get()->GetAllThreadObjects();
    for (auto& thread : threads) {
        if (thread->StateValueExists<ScriptSystemVM>()) {
            auto vm = thread->GetStateValue<ScriptSystemVM>();
            vm->InvalidateConstructionScript(script_path);
        }
    }
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

void ScriptSystemManager::OnCollision(CollisionEvent* col_event)
{
    Entity ent = col_event->reciever;
    
    if (!Application::GetWorld().HasComponentSynced<ScriptComponent>(ent)) {
        SceneNode* node = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(ent);
        if (node && (bool)(node->state & SceneNodeState::PREFAB_CHILD)) {
            while (!(bool)(node->state & SceneNodeState::PREFAB))
            {
                if (!node) return;
                node = node->parent;
            }
            ent = node->entity;
            if (!Application::GetWorld().HasComponentSynced<ScriptComponent>(ent)) return;
        }
        else {
            return;
        }
    }

    auto fnd = entity_collisions.find(ent.id);
    if (fnd == entity_collisions.end()) {
        collided_entities.push_back(ent);
        fnd = entity_collisions.insert(std::make_pair(ent.id, std::vector<CollisionEvent_L>())).first;
    } 

    CollisionEvent_L cl;
    cl.collider = entity{ col_event->other.id };
    cl.num_collision_points = col_event->collision_point_number;
    for (int i = 0; i < cl.num_collision_points; i++) {
        cl.collision_points[i] = *reinterpret_cast<vec3*>(& col_event->collision_points[i]);
    }
    fnd->second.push_back(cl);
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

ScriptSystemManager::ScriptSystemManager() : sync_mutex(), m_DefferedSetMaps(), DefferedSetMaps_mutex(), m_ScriptCache(), m_Deffered_call_maps(), Deffered_call_maps_mutex(), m_Pending_Deffered_call_vectors(),
    collision_observer(nullptr)
{
    m_Deffered_call_maps.emplace_back();
    m_Deffered_call_maps.emplace_back();
    m_Pending_Deffered_call_vectors.emplace_back();
    m_Pending_Deffered_call_vectors.emplace_back();
    m_DefferedSetMaps.reserve(ThreadManager::Get()->GetMaxThreadCount());
    collision_observer.reset(MakeEventObserver<CollisionEvent>([this](CollisionEvent* collision) {
        OnCollision(collision);
        return false;
        }));
    Application::Get()->RegisterObserver<CollisionEvent>(collision_observer.get());

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

void ScriptSystemVM::RunGarbageCollector()
{
    m_LuaEngine.RunGarbageCollector();
    m_LuaInitializationEngine.RunGarbageCollector();
}

void ScriptSystemVM::SetEngineInitializationEntity(Entity ent, const std::string& path)
{
    current_Initialization_handler = InitializationScriptHandler(ent, FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(path)));
    m_LuaInitializationEngine.SetClassInstance(&current_Initialization_handler);
    init_mode = true;
}

void ScriptSystemVM::InvalidateInlineScript(const std::string& script_path)
{
    auto hash = LuaEngineUtilities::ScriptHash(script_path, false);
    auto fnd = m_BoundScripts.find(hash);
    if (fnd != m_BoundScripts.end()) {
        m_BoundScripts.erase(hash);
    }
}

void ScriptSystemVM::InvalidateConstructionScript(const std::string& script_path)
{
    auto hash = LuaEngineUtilities::ScriptHash(script_path, true);
    auto fnd = m_BoundInitializationScripts.find(hash);
    if (fnd != m_BoundInitializationScripts.end()) {
        m_BoundInitializationScripts.erase(hash);
    }
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


    IOModule().RegisterModule(props);
    TimeModule().RegisterModule(props);
    DefferedPropertySetModule().RegisterModule(props);
    ApplicationDataModule().RegisterModule(props);
    LocalEntityModule().RegisterModule(props);
    LocalPropertySetModule().RegisterModule(props);
    PrefabManipulationModule().RegisterModule(props);
    RayCastingModule().RegisterModule(props);
    CollisionModule().RegisterModule(props);

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
    });

    IOModule().RegisterModule(props);
    TimeModule().RegisterModule(props);
    ApplicationDataModule().RegisterModule(props);
    LocalPropertySetModule().RegisterModule(props);
    EventModule().RegisterModule(props);

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
