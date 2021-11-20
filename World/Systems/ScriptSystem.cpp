#include "ScriptSystem.h"
#include <algorithm>
#include <FileManager.h>
#include <string>
#include <World/Components/SquareComponent.h>
#include <sstream>
#include <World/World.h>
#include <Application.h>
#include <fstream>
#include <ThreadManager.h>
#include <stdexcept>

ScriptSystemManager* ScriptSystemManager::instance = nullptr;

std::string ScriptHash(std::string script_path)
{
    std::replace(script_path.begin(), script_path.end(), '/', '_');
    auto first = script_path.find_first_of('.', 0);

    return "Object_" + script_path.substr(0,first);
}

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
    auto file = m_ScriptCache.find(ScriptHash(path));
    if (file != m_ScriptCache.end()) {
        return (*file).second.script;
    }
    else {
        return LoadScript(path);
    }
}

std::string& ScriptSystemManager::LoadScript(const std::string& path)
{
    std::ifstream file_in(FileManager::Get()->GetAssetFilePath(path));
    if (file_in.is_open()) {
        std::stringstream stream;
        stream << file_in.rdbuf();
        std::string file = stream.str();
        std::string script = ParseScript(file,ScriptHash(path));
        auto out = m_ScriptCache.insert(std::make_pair(ScriptHash(path), ScriptObject(script)));
        return (*(out.first)).second.script;
    }
    else {
        throw std::runtime_error("Script couldn't be opened");
    }
}

std::string ScriptSystemManager::ParseScript(std::string script, const std::string& hash)
{
    size_t pos = 0;
    const char * delim = "( \n\t";
    while (pos != script.npos) {
        auto find_res = script.find("function", pos);
        if (find_res == script.npos) break;
        auto space_it = script.find_first_of(delim, find_res);
        auto begin_second_word = script.find_first_not_of(delim, space_it);
        auto end_second_word = script.find_first_of(delim, begin_second_word);
        std::string replacement = hash + ":" + script.substr(begin_second_word, end_second_word - begin_second_word);
        script.replace(script.begin() + begin_second_word, script.begin() + end_second_word, replacement);
        pos = begin_second_word + replacement.size();
    }
    std::stringstream stream;
    stream << hash << " = {}\n";

    return stream.str() + script;
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

void ScriptHandler::BindHandlerFunctions(LuaEngineClass<ScriptHandler>* script_engine)
{
    std::vector<LuaEngineClass<ScriptHandler>::LuaEngine_Function_Binding> bindings{
        //This is where function bindings go
        {"MoveSquare" ,LuaEngineClass<ScriptHandler>::InvokeClass<&ScriptHandler::TestChangeSquarePos>}

    };
    if (bindings.empty()) return;
    script_engine->AddBindings(bindings);
}

void ScriptHandler::TestChangeSquarePos(float x, float y)
{
    Application::GetWorld().GetComponent<SquareComponent>(current_entity).pos += glm::vec2(x, y);
}
