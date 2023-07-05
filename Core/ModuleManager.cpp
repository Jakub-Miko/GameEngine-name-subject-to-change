#include "ModuleManager.h"
#include <FileManager.h>
#include <filesystem>
#include <Application.h>

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef UNIX
#include <dlfcn.h>
#endif

#ifdef ENGINE_CORE

extern "C" LIBEXP module_traits CDECL_CALL InitModule() {
	throw std::runtime_error("This function is just a template and should be implemented within a module");
}

extern "C" LIBEXP module_types CDECL_CALL GetTypes_dummy() {
	throw std::runtime_error("This function is just a template and should be implemented within a module");
}

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
	
#ifdef WIN32
	module->lib = (void*)new HMODULE(lib);
#elif defined(UNIX)
	module->lib = lib;
#endif

	auto module_init_func = module->GetSymbol<decltype(InitModule)>("InitModule");

	if (!module_init_func) {
		throw std::runtime_error("Module " + module_name + " could not be loaded, because it doesnt containt a InitModule function");
	}

	ModuleLoadEvent* event = new ModuleLoadEvent;
	event->module_name = module_name;
	event->module = module;
	event->module_traits = module_init_func().module_trait_list;
	if (event->module_traits.empty()) {
		throw std::runtime_error("Module " + module_name + " could not be loaded, because it doesnt containt any traits");
	}
	Application::Get()->SendObservedEvent<ModuleLoadEvent>(event);
	std::lock_guard<std::mutex> lock2(module_factory_mutex);
	for (auto& factory : module_factories) {
		factory->ProcessLoad(event);
	}
	delete event;

	module_list.insert(std::make_pair(module_name, module));
	return module;

}

void ModuleManager::UnloadModule(const std::string& module_name)
{
	std::lock_guard<std::mutex> lock(module_list_mutex);
	auto fnd = module_list.find(module_name);
	if (fnd != module_list.end()) {
		ModuleUnLoadEvent* event = new ModuleUnLoadEvent;
		event->module_name = module_name;
		event->module = fnd->second;
		Application::Get()->SendObservedEvent<ModuleUnLoadEvent>(event);
		std::lock_guard<std::mutex> lock2(module_factory_mutex);
		for (auto& factory : module_factories) {
			factory->ProcessUnload(event);
		}
		delete event;
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
	ModuleResetEvent* event = new ModuleResetEvent;
	Application::Get()->SendObservedEvent<ModuleResetEvent>(event);
	std::lock_guard<std::mutex> lock2(module_factory_mutex);
	for (auto& factory : module_factories) {
		factory->ProcessReset();
	}
	delete event;
	module_list.clear();
}

ModuleManager::ModuleManager() : module_list(), module_factory_mutex(), module_list_mutex()
{
	
}

void BaseModuleFactory::RegisterFactory(BaseModuleFactory* instance)
{
	auto manager = ModuleManager::Get();
	std::lock_guard<std::mutex> lock(manager->module_factory_mutex);
	manager->module_factories.push_back(std::unique_ptr<BaseModuleFactory>(instance));
}
