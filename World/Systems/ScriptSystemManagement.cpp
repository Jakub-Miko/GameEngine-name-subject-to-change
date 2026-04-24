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
#include <World/Components/DeferredUpdateComponent.h>
#include <World/EntityManager.h>
#include <sstream>
#include <World/World.h>
#include <Application.h>
#include <fstream>
#include <ThreadManager.h>
#include <Events/CollisionEvent.h>
#include <World/ScriptModules/IOModule.h>
#include <World/ScriptModules/DeferredPropertySetModule.h>
#include <World/ScriptModules/ApplicationDataModule.h>
#include <World/ScriptModules/CollisionModule.h>
#include <World/ScriptModules/LocalPropertySetModule.h>
#include <World/ScriptModules/TimeModule.h>
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
    ThreadManager::Get()->SetThreadLocalData<Deferred_Set_Map>(GetDeferredSetMap());
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
    Application::GetWorld().SetComponent<DeferredUpdateComponent>(ent);
}

std::shared_ptr<Deferred_Call_Map> ScriptSystemManager::GetDeferredCalls()
{
    return m_Deferred_call_maps[deferred_call_cycle ? 1 : 0];
}

std::vector<Deferred_Call>& ScriptSystemManager::GetDeferredCallsForEntity(Entity ent)
{
    auto fnd = GetDeferredCalls()->find(ent.id);
    if (fnd != GetDeferredCalls()->end()) {
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

std::vector<Entity>& ScriptSystemManager::GetPendingDeferredCallEntities()
{
    return m_Pending_Deferred_call_vectors[deferred_call_cycle ? 1 : 0];
}

void ScriptSystemManager::AddDeferredCall(Entity ent, const Deferred_Call& call_info)
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


    std::lock_guard<std::mutex> lock(Deferred_call_maps_mutex);
    auto& call_map = m_Deferred_call_maps[deferred_call_cycle ? 0 : 1];
    auto& call_entities = m_Pending_Deferred_call_vectors[deferred_call_cycle ? 0 : 1];
    auto fnd = call_map->find(ent.id);
    if (fnd != call_map->end()) {
        fnd->second.push_back(call_info);
    }
    else {
        call_entities.push_back(ent);
        auto new_entry = call_map->insert(std::make_pair(ent.id, std::vector<Deferred_Call>()));
        new_entry.first->second.push_back(call_info);
    }
}
 
void ScriptSystemManager::SwapDeferredCallCycle()
{
    deferred_call_cycle = !deferred_call_cycle;
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

const std::vector<std::shared_ptr<Deferred_Set_Map>>& ScriptSystemManager::GetEntityChanges()
{
    std::lock_guard<std::mutex> lock(DeferredSetMaps_mutex);
    return m_DeferredSetMaps;
} 

void ScriptSystemManager::ClearEntityChanges()
{
    for (auto& map : m_DeferredSetMaps) {
        map->clear();
    }
}

std::shared_ptr<ScriptSystemVM> ScriptSystemManager::TryGetScriptSystemVM()
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
        auto vm = std::make_shared<ScriptSystemVM>();
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

}

ScriptSystemManager::ScriptSystemManager() : sync_mutex(), m_DeferredSetMaps(), DeferredSetMaps_mutex(), m_ScriptCache(), m_Deferred_call_maps(), Deferred_call_maps_mutex(), m_Pending_Deferred_call_vectors(),
    collision_observer(nullptr)
{
    m_Deferred_call_maps.emplace_back();
    m_Deferred_call_maps.emplace_back();
    m_Pending_Deferred_call_vectors.emplace_back();
    m_Pending_Deferred_call_vectors.emplace_back();
    m_DeferredSetMaps.reserve(ThreadManager::Get()->GetMaxThreadCount());
    collision_observer.reset(MakeEventObserver<CollisionEvent>([this](CollisionEvent* collision) {
        OnCollision(collision);
        return false;
        }));
    Application::Get()->RegisterObserver<CollisionEvent>(collision_observer.get());

}

std::shared_ptr<Deferred_Set_Map> ScriptSystemManager::GetDeferredSetMap()
{
    std::lock_guard<std::mutex> lock(DeferredSetMaps_mutex);
    if ((m_DeferredSetMaps.size() + 1) > m_DeferredSetMaps.capacity()) {
        throw std::runtime_error("Invalid number of deffred set maps allocated");
    }
    auto map = std::make_shared<Deferred_Set_Map>();
    m_DeferredSetMaps.push_back(map);
    return map;
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
    current_Initialization_handler = InitializationScriptHandler(ent, FileManager::Get()->GetPath(path));
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
    std::vector<std::pair<std::string, std::string>> optional_dlls = { {"Engine",FileManager::Get()->GetLibraryPath("EngineCore")} };
    script_engine->InitFFI(optional_dlls);


    IOModule().RegisterModule(props);
    TimeModule().RegisterModule(props);
    DeferredPropertySetModule().RegisterModule(props);
    ApplicationDataModule().RegisterModule(props);
    LocalEntityModule().RegisterModule(props);
    LocalPropertySetModule().RegisterModule(props);
    PrefabManipulationModule().RegisterModule(props);
    CollisionModule().RegisterModule(props);

    script_engine->RegisterModule(props);
}

void InitializationScriptHandler::BindHandlerFunctions(LuaEngineClass<InitializationScriptHandler>* script_engine)
{
    ModuleBindingProperties props;
    std::vector<std::pair<std::string, std::string>> optional_dlls = { {"Engine",FileManager::Get()->GetLibraryPath("EngineCore")} };
    script_engine->InitFFI(optional_dlls);

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
