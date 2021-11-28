#include "ScriptSystemManagement.h"
#include <algorithm>
#include <Input/Input.h>
#include <Events/KeyCodes.h>
#include <FileManager.h>
#include <string>
#include <World/Components/SquareComponent.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <sstream>
#include <World/World.h>
#include <Application.h>
#include <fstream>
#include <ThreadManager.h>
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

std::string& ScriptSystemManager::GetScript(const std::string& path)
{
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

ScriptSystemManager::ScriptSystemManager() : sync_mutex()
{

}

ScriptSystemVM::ScriptSystemVM() : m_LuaEngine(), curentHandler(Entity())
{
    ScriptHandler::BindKeyCodes(&m_LuaEngine);
    ScriptHandler::BindHandlerFunctions(&m_LuaEngine);
}

ScriptSystemVM::~ScriptSystemVM()
{
}

void ScriptSystemVM::SetEngineEntity(Entity ent)
{
    curentHandler = ScriptHandler(ent);
    m_LuaEngine.SetClassInstance(&curentHandler);
}

void ScriptSystemVM::ResetScriptVM()
{
    m_LuaEngine = LuaEngineClass<ScriptHandler>();
    m_BoundScripts.swap(std::unordered_set<std::string>());
    curentHandler = ScriptHandler(Entity());
    ScriptHandler::BindKeyCodes(&m_LuaEngine);
    ScriptHandler::BindHandlerFunctions(&m_LuaEngine);
}

void ScriptHandler::BindHandlerFunctions(LuaEngineClass<ScriptHandler>* script_engine)
{
    std::vector<LuaEngineClass<ScriptHandler>::LuaEngine_Function_Binding> bindings{
        //This is where function bindings go
        {"MoveSquare" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::TestChangeSquarePos>},
        {"GetPos" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::TestGetPosition>},
        {"GetProperty_INT" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::GetProperty<int>>},
        {"GetProperty_FLOAT" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::GetProperty<float>>},
        {"GetProperty_STRING" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::GetProperty<std::string>>},
        {"GetProperty_VEC2" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::GetProperty<glm::vec2>>},
        {"GetProperty_VEC3" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::GetProperty<glm::vec3>>},
        {"SetProperty_INT" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::SetProperty<int>>},
        {"SetProperty_FLOAT" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::SetProperty<float>>},
        {"SetProperty_VEC2" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::SetProperty<glm::vec2>>},
        {"SetProperty_VEC3" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::SetProperty<glm::vec3>>},
        {"SetProperty_STRING" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::SetProperty<std::string>>},
        {"PropertyExists" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::PropertyExists>},
        {"IsKeyPressed" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::IsKeyPressed>},
        {"IsMouseButtonPressed" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::IsMouseButtonPressed>},
        {"GetMousePosition", LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::GetMousePosition>},
        {"EnableKeyPressedEvents", LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::EnableKeyPressedEvents>},
        {"EnableMouseButtonPressedEvents", LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::EnableMouseButtonPressedEvents>}

    };
    if (bindings.empty()) return;
    script_engine->AddBindings(bindings);
}

void ScriptHandler::BindKeyCodes(LuaEngineClass<ScriptHandler>* script_engine)
{
    script_engine->RunString(ScriptKeyBindings);
}

void ScriptHandler::TestChangeSquarePos(float x, float y)
{
    Application::GetWorld().GetComponent<TransformComponent>(current_entity).TransformMatrix[3] = glm::vec4(x, y, 0.0f,1.0f);
}

glm::vec2 ScriptHandler::TestGetPosition()
{
    return Application::GetWorld().GetComponent<TransformComponent>(current_entity).TransformMatrix[3];
}

bool ScriptHandler::PropertyExists(std::string name)
{
    auto& props = Application::GetWorld().GetComponent<ScriptComponent>(current_entity).m_Properties;
    auto find = props.find(name);
    if (find != props.end()) {
        return true;
    };
    return false;
}

bool ScriptHandler::IsKeyPressed(int key_code)
{
    return Input::Get()->IsKeyPressed((KeyCode)key_code);
}

bool ScriptHandler::IsMouseButtonPressed(int key_code)
{
    return Input::Get()->IsMouseButtonPressed((MouseButtonCode)key_code);
}

glm::vec2 ScriptHandler::GetMousePosition()
{
    return Input::Get()->GetMoutePosition();
}

void ScriptHandler::EnableKeyPressedEvents()
{
    Application::GetWorld().SetComponent<KeyPressedScriptComponent>(current_entity);
}

void ScriptHandler::EnableMouseButtonPressedEvents()
{
    Application::GetWorld().SetComponent<MousePressedScriptComponent>(current_entity);
}

