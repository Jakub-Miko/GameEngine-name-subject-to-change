#pragma once 
#include <World/Components/MousePressedScriptComponent.h>
#include <World/System.h>
#include <Events/MouseButtonPressEvent.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/ScriptComponent.h>

inline void MousePressedScriptSystem(World& world, MouseButtonPressEvent* e) {
    auto func_1 = [&world,&e](ComponentCollection compcol, system_view_type<MousePressedScriptComponent>& comps, entt::registry* reg) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }
        
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            if (reg->all_of<ScriptComponent>(*iter)) {
                auto comp = reg->get<ScriptComponent>(*iter);
                script_vm->SetEngineEntity(Entity((uint32_t)*iter));
                script_vm->CallFunction(comp.script_path, "OnMouseButtonPressed", *e);
            }
        };

    };

    RunSystemSimple<MousePressedScriptComponent>(world, func_1);
}
