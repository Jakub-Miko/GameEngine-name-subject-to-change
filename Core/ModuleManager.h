#pragma once 
#include <Core/ModuleInterface.h>
#include <Events/Event.h>
#include <Core/Defines.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <mutex>


class Module {
private:
    void* GetSymbol_internal(const std::string& symbol_name);
public:
    Module(const Module& other) = delete;
    Module& operator=(const Module& other) = delete;
    ~Module();

    template<typename T = void>
    T* GetSymbol(const std::string& symbol_name) {
        T* symbol = (T*)GetSymbol_internal(symbol_name);
        if (!symbol) {
            throw std::runtime_error("Symbol " + symbol_name + " from module " + module_name + " could not be loaded");
        }
        return symbol;
    }

    template<typename T = void>
    bool TryGetSymbol(const std::string& symbol_name, T** symbol_pointer) {
        T* symbol = (T*)GetSymbol_internal(symbol_name);
        if (!symbol) {
            return false;
        }
        *symbol_pointer = symbol;
        return true;
    }

    const std::string& GetModuleName() const;

private:
    Module();

    friend class ModuleManager;

    std::string module_name;
    void* lib;
};

class ModuleLoadEvent : public Event {
    EVENT_ID(ModuleLoadEvent)
public:
    ModuleLoadEvent() = default;
    ModuleLoadEvent(const ModuleLoadEvent& other) = default;
    std::string module_name;
    std::vector<std::string> module_traits;
    std::shared_ptr<Module> module;
};

class ModuleUnLoadEvent : public Event {
    EVENT_ID(ModuleUnLoadEvent)
public:
    ModuleUnLoadEvent() = default;
    ModuleUnLoadEvent(const ModuleUnLoadEvent& other) = default;
    std::string module_name;
    std::shared_ptr<Module> module;
};

class ModuleResetEvent : public Event {
    EVENT_ID(ModuleResetEvent)
public:
    ModuleResetEvent() = default;
    ModuleResetEvent(const ModuleResetEvent& other) = default;
};

class ModuleManager {
public:
    ModuleManager(const ModuleManager& ref) = delete;
    ModuleManager(ModuleManager&& ref) = delete;
    ModuleManager& operator=(const ModuleManager& ref) = delete;
    ModuleManager& operator=(ModuleManager&& ref) = delete;

    static ModuleManager* Get();
    static void Init();
    static void Shutdown();

    std::shared_ptr<Module> GetModule(const std::string& module_name);
    std::shared_ptr<Module> LoadModule(const std::string& module_name);
    void UnloadModule(const std::string& module_name);
    std::vector<std::string> GetLoadedModules();
    void UnloadAllModules();

private:
    ModuleManager();
    ~ModuleManager() = default;
    static ModuleManager* instance;
    friend class BaseModuleFactory;
    std::unordered_map<std::string,std::shared_ptr<Module>> module_list;
    std::vector<std::unique_ptr<BaseModuleFactory>> module_factories;
    std::mutex module_list_mutex;
    std::mutex module_factory_mutex;
};

class BaseModuleFactory {
protected:
    friend ModuleManager;
    static void RegisterFactory(BaseModuleFactory* instance);
    virtual void ProcessLoad(ModuleLoadEvent* event) = 0;
    virtual void ProcessReset() = 0;
    virtual void ProcessUnload(ModuleUnLoadEvent* event) = 0;
    BaseModuleFactory() {}
public:
    virtual ~BaseModuleFactory() {}
};

//Only one instance of this is going to exist and its going to be owned by ModuleManager
template<typename T,
    typename Dummy = std::enable_if_t<std::is_same_v<decltype(std::declval<T>().clone()),T*>>, 
    typename Dummy2 = std::enable_if_t<RuntimeTag<T>::IsIdAssigned()>>
class ModuleFactory : public BaseModuleFactory {
public:

    ModuleFactory(const ModuleFactory& ref) = delete;
    ModuleFactory(ModuleFactory&& ref) = delete;
    ModuleFactory& operator=(const ModuleFactory& ref) = delete;
    ModuleFactory& operator=(ModuleFactory&& ref) = delete;

    //This method is not thread-safe
    static void Initialize() {
        if (!instance) {
            auto new_instance = new ModuleFactory<T>;
            RegisterFactory(new_instance);
            instance = new_instance;
        }
    }

    virtual ~ModuleFactory() {
        instance = nullptr;
    }

    static ModuleFactory<T>* Get() {
        return instance;
    }

    T* CreateType(const std::string& type_name) {
        std::lock_guard<std::mutex> lock(base_type_mutex);
        auto fnd = base_types.find(type_name);
        if (fnd != base_types.end()) {
            return fnd->second->clone();
        }
        else {
            throw std::runtime_error("Type " + type_name + "could not be found");
        }
    }

    std::vector<std::string> GetTypeList() {
        std::lock_guard<std::mutex> lock(base_type_mutex);
        std::vector<std::string> types;
        types.reserve(base_types.size());
        for (auto type_name : base_types) {
            types.push_back(type_name->first);
        }
        return std::move(types);
    }

    typedef module_types(*construction_function_ptr)();

    struct module_type_binding {
        std::vector<std::string> loaded_types;
    };
    virtual void ProcessLoad(ModuleLoadEvent* event) override {
        for (auto& trait : event->module_traits) {
            if (trait == RuntimeTag<T>::GetName()) {
                auto types_func = (construction_function_ptr)event->module->GetSymbol(std::string("GetTypes_") + std::string(RuntimeTag<T>::GetName()));
                auto types = types_func();
                std::lock_guard<std::mutex> lock(base_type_mutex);
                std::vector<std::string> type_bindings;
                for (auto& type : types.types) {
                    base_types.insert(std::make_pair(type.first, (T*)type.second));
                    type_bindings.push_back(type.first);
                }
                module_type_bindings.insert(std::make_pair(event->module_name, module_type_binding{ std::move(type_bindings) }));
                return;
            }
        }
    }

    virtual void ProcessReset() override {
        std::lock_guard<std::mutex> lock(base_type_mutex);
        base_types.clear();
        module_type_bindings.clear();
    }

    virtual void ProcessUnload(ModuleUnLoadEvent* event) override {
        std::lock_guard<std::mutex> lock(base_type_mutex);
        auto fnd = module_type_bindings.find(event->module_name);
        if (fnd != module_type_bindings.end()) {
            for (auto& type : fnd->second.loaded_types) {
                base_types.erase(type);
            }
            module_type_bindings.erase(event->module_name);
        }
        return;
    }

private:
    ModuleFactory() : base_types(), base_type_mutex(){

    }

    static inline ModuleFactory<T>* instance = nullptr;
    std::unordered_map<std::string, const std::unique_ptr<T>> base_types;
    std::unordered_map<std::string, module_type_binding> module_type_bindings;
    std::mutex base_type_mutex;
};

