#include "EntityConstructionSystem.h"
#include <World/System.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/EntityManager.h>

void EntityConstructionSystem(World& world)
{
    auto func_1 = [&world](ComponentCollection compcol, system_view_type<ConstructionComponent>& comps, entt::registry* reg) {
        
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }
        
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            PROFILE("CONSTRUCT ENTITY");
            auto comp = world.GetComponent<ConstructionComponent>(*iter);
            EntityParseResult entity_template = EntityManager::Get()->GetEntitySignature(comp.path);
            Entity new_ent = *iter;
            
            if (world.HasComponentSynced<LoadedComponent>(new_ent)) {
                script_vm->SetEngineInitializationEntity(new_ent, comp.path);
                script_vm->CallInitializationFunction(comp.path, "OnConstruct");
                world.SetComponent<InitializationComponent>(new_ent);
            }
            else {
                world.SetComponent<LoadedComponent>(new_ent, LoadedComponent(comp.path));
                world.CreateEntityFromEmpty(new_ent, comp.parent);
                script_vm->SetEngineInitializationEntity(new_ent,comp.path);
                world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
                script_vm->CallInitializationFunction(comp.path, "OnConstruct");
                world.SetComponent<InitializationComponent>(new_ent);
            }

            for (auto child : entity_template.children) {
                EntityParseResult child_entity_template = EntityManager::Get()->GetEntitySignature(child);
                Entity child_ent = world.CreateEntity(new_ent);
                script_vm->SetEngineInitializationEntity(child_ent, child);
                world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
                script_vm->CallInitializationFunction(child, "OnConstruct");
                world.SetComponent<InitializationComponent>(child_ent);
            }

        }
    };

    RunSystemSimple<ConstructionComponent>(world, func_1);
    world.GetRegistry().clear<ConstructionComponent>();
}
