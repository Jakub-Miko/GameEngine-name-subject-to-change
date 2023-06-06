#pragma once 
#include <Core/Defines.h>
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

    template<typename T>
    T* GetSymbol(const std::string& symbol_name) {
        T* symbol = (T*)GetSymbol_internal(symbol_name);
        if (!symbol) {
            throw std::runtime_error("Symbol " + symbol_name + " from module " + library_path + " could not be loaded");
        }
        return symbol;
    }

    template<typename T>
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
    ModuleManager() : module_list() {}
    ~ModuleManager() = default;
    static ModuleManager* instance;
    std::unordered_map<std::string,std::shared_ptr<Module>> module_list;
    std::mutex module_list_mutex;
};