#include "ScriptSystem.h"
#include "ScriptSystemManagement.h"

void ScriptSystem(World& world, float delta_time)
{
    {
        auto func_1 = [&delta_time](ComponentCollection compcol, system_view_type<ScriptComponent>& comps, entt::registry* reg) {
            PROFILE("ScriptRunThread");
            auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
            if (!script_vm) {
                ScriptSystemManager::Get()->InitializeScriptSystemVM();
                script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
            }

            for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
                auto comp = reg->get<ScriptComponent>(*iter);
                script_vm->SetEngineEntity(Entity((uint32_t)*iter));
                script_vm->CallFunction(comp.script_path, "OnUpdate", delta_time);

            };

        };

        RunSystemSimple<ScriptComponent>(world, func_1);
    }
}
