#include "ScriptSystem.h"
#include "ScriptSystemManagement.h"
#include <World/Components/DefferedUpdateComponent.h>

void ScriptSystemUpdate(World& world, float delta_time)
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

void ScriptSystemDefferedSet(World& world)
{
    auto func_deffered = [&world](ComponentCollection compcol, system_view_type<DefferedUpdateComponent>& comps, entt::registry* reg) {

        auto& changes = ScriptSystemManager::Get()->GetEntityChanges();

        for (auto iter = comps.begin() + compcol.start_index; iter != comps.begin() + compcol.start_index + compcol.size; iter++) {
            auto& dynamic_prop = world.GetRegistry().get<DynamicPropertiesComponent>((entt::entity)(*iter));
            for (auto& change : changes) {
                auto fnd = change.find((uint32_t)(*iter));
                if (fnd != change.end()) {
                    for (auto& changed_props : fnd->second) {
                        dynamic_prop.m_Properties.insert_or_assign(changed_props.name, changed_props.value);
                    }
                }
            }

        }



    };

    RunSystemSimple<DefferedUpdateComponent>(world, func_deffered);
    world.GetRegistry().clear<DefferedUpdateComponent>();
    ScriptSystemManager::Get()->ClearEntityChanges();
}
