#include "ModuleManager.h"
#include <FileManager.h>
#include <filesystem>

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef defined(UNIX)
#include <dlfcn.h>
#endif

ModuleManager* ModuleManager::instance = nullptr;

Module::~Module()
{
#ifdef UNIX
	dlclose(lib);
#endif
#ifdef WIN32
	FreeLibrary(*(HMODULE*)lib);
	delete (HMODULE*)lib;
#endif
}

const std::string& Module::GetModuleName() const
{
	return module_name;
}

Module::Module() : lib(nullptr), module_name("Unknown")
{

}

void* Module::GetSymbol_internal(const std::string& symbol_name)
{
#ifdef WIN32
	void* symbol = (void*)GetProcAddress(*(HMODULE*)lib, symbol_name.c_str());
#elif defined(UNIX)
	void* symbol = (void*)dlsym(lib, symbol_name.c_str());
#endif
	return symbol;
}

ModuleManager* ModuleManager::Get()
{
	return instance;
}

void ModuleManager::Init()
{
	if (!instance) {
		instance = new ModuleManager;
	}
}

void ModuleManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

std::shared_ptr<Module> ModuleManager::GetModule(const std::string& module_name)
{
	std::lock_guard<std::mutex> lock(module_list_mutex);
	auto fnd = module_list.find(module_name);
	if (fnd != module_list.end()) {
		return fnd->second;
	}

	return nullptr;
}

std::shared_ptr<Module> ModuleManager::LoadModule(const std::string& module_name)
{
	std::lock_guard<std::mutex> lock(module_list_mutex);
	auto fnd = module_list.find(module_name);
	if (fnd != module_list.end()) {
		return fnd->second;
	}

	auto path = FileManager::Get()->GetLibraryPath(module_name);
#ifdef WIN32
	HMODULE lib = LoadLibraryA(path.c_str());
#elif defined(UNIX)
	void* lib = dlopen(path.c_str(), RTLD_LAZY);
#endif
	if (!lib) {
		throw std::runtime_error("Module " + module_name + " could not be opened.");
	}
	
	std::shared_ptr<Module> module = std::shared_ptr<Module>(new Module);
	module->module_name = module_name;
	module->lib = (void*)new HMODULE(lib);

	module_list.insert(std::make_pair(module_name, module));
	return module;

}

void ModuleManager::UnloadModule(const std::string& module_name)
{
	std::lock_guard<std::mutex> lock(module_list_mutex);
	auto fnd = module_list.find(module_name);
	if (fnd != module_list.end()) {
		module_list.erase(module_name);
	}
}

std::vector<std::string> ModuleManager::GetLoadedModules()
{
	std::vector<std::string> modules;
	std::lock_guard<std::mutex> lock(module_list_mutex);
	modules.reserve(module_list.size());
	for (auto module : module_list) {
		modules.push_back(module.first);
	}
	return modules;
}

void ModuleManager::UnloadAllModules()
{
	std::lock_guard<std::mutex> lock(module_list_mutex);
	module_list.clear();
}
