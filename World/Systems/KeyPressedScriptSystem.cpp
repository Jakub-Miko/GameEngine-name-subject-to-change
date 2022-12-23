#include "KeyPressedScriptSystem.h"
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/System.h>
#include <Events/KeyPressEvent.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/ScriptComponent.h>


void KeyPressedScriptSystem(World& world, KeyPressedEvent* e) {
    auto func_1 = [&world, &e](ComponentCollection compcol, system_view_type<KeyPressedScriptComponent>& comps, entt::registry* reg) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }

        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            if (reg->all_of<ScriptComponent>(*iter)) {
                auto comp = reg->get<ScriptComponent>(*iter);
                script_vm->SetEngineEntity(Entity((uint32_t)*iter));
                script_vm->CallFunction(comp.script_path, "OnKeyPressed", *e);
            }
        };

    };

    RunSystemSimple<KeyPressedScriptComponent>(world, func_1);
}
