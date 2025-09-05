#include "ScriptSystem.h"
#include "ScriptSystemManagement.h"
#include <World/Components/DeferredUpdateComponent.h>

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

void ScriptSystemDeferredSet(World& world)
{
    auto func_deferred = [&world](ComponentCollection compcol, system_view_type<DeferredUpdateComponent>& comps, entt::registry* reg) {

        auto& changes = ScriptSystemManager::Get()->GetEntityChanges();

        for (auto iter = comps.begin() + compcol.start_index; iter != comps.begin() + compcol.start_index + compcol.size; iter++) {
            auto& dynamic_prop = world.GetRegistry().get<DynamicPropertiesComponent>((entt::entity)(*iter));
            for (auto change : changes) {
                auto fnd = change->find((uint32_t)(*iter));
                if (fnd != change->end()) {
                    for (auto& changed_props : fnd->second) {
                        dynamic_prop.m_Properties.insert_or_assign(changed_props.name, changed_props.value);
                    }
                }
            }

        }



    };

    RunSystemSimple<DeferredUpdateComponent>(world, func_deferred);
    world.GetRegistry().clear<DeferredUpdateComponent>();
    ScriptSystemManager::Get()->ClearEntityChanges();
}

void ScriptSystemDeferredCall(World& world)
{
    ScriptSystemManager::Get()->SwapDeferredCallCycle();
    auto* current_entities = &ScriptSystemManager::Get()->GetPendingDeferredCallEntities();
    auto current_calls = ScriptSystemManager::Get()->GetDeferredCalls();
    
    auto func_deferred = [&world](ComponentCollection compcol, const std::vector<Entity>& comps) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }

        for (auto iter = comps.begin() + compcol.start_index; iter != comps.begin() + compcol.start_index + compcol.size; iter++) {
            auto& changes = ScriptSystemManager::Get()->GetDeferredCallsForEntity(*iter);
            auto& script_comp = world.GetComponent<ScriptComponent>(*iter);
            for (auto& change : changes) {
                script_vm->SetEngineEntity(*iter);
                script_vm->TryCallFunctionRuntime(nullptr, script_comp.script_path, change.func_name, change.arguments);
            }
        }



    };
    while (!current_entities->empty()) {
        RunSystemSimpleVector(world, *current_entities, func_deferred);
        current_calls->clear();
        current_entities->clear();
        ScriptSystemManager::Get()->SwapDeferredCallCycle();
        current_entities = &ScriptSystemManager::Get()->GetPendingDeferredCallEntities();
        current_calls = ScriptSystemManager::Get()->GetDeferredCalls();
    }

}

void ScriptSystemCollisionCallback(World& world) {
    auto* current_entities = &ScriptSystemManager::Get()->GetCollidedEntities();
    auto* current_collisions = &ScriptSystemManager::Get()->GetEntityCollisions();

    auto func_deferred = [&world](ComponentCollection compcol, const std::vector<Entity>& comps) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }

        for (auto iter = comps.begin() + compcol.start_index; iter != comps.begin() + compcol.start_index + compcol.size; iter++) {
            auto& col = ScriptSystemManager::Get()->GetEntityCollisions(*iter);
            auto& script_comp = world.GetComponent<ScriptComponent>(*iter);
            for (auto& collision : col) {
                script_vm->SetEngineEntity(*iter);
                script_vm->TryCallFunction(nullptr, script_comp.script_path, "OnCollision", collision);
            }
        }



    };
    RunSystemSimpleVector(world, *current_entities, func_deferred);
    current_collisions->clear();
    current_entities->clear();
}