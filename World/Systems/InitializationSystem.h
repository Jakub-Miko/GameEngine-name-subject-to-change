#pragma once 

#include <World/Components/InitializationComponent.h>
#include <World/System.h>
#include <World/Systems/ScriptSystem.h>
#include <World/Components/ScriptComponent.h>

inline void InitializationSystem(World& world) {
    auto func_1 = [&world](ComponentCollection compcol, system_view_type<InitializationComponent>& comps, entt::registry* reg) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }
        
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            if (reg->all_of<ScriptComponent>(*iter)) {
                auto comp = reg->get<ScriptComponent>(*iter);
                script_vm->SetEngineEntity(Entity((uint32_t)*iter));
                script_vm->TryCallFunction(nullptr,comp.script_path, "OnConstruct");
                
            }
        };

    };

    RunSystemSimple<InitializationComponent>(world, func_1);
    world.GetRegistry().clear<InitializationComponent>();
}