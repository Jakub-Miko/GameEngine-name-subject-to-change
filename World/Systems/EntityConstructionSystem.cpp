#include "EntityConstructionSystem.h"
#include <World/System.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/EntityManager.h>

void EntityConstructionSystem(World& world)
{
    auto func_1 = [&world](ComponentCollection compcol, std::deque<Construction_Entry>& queue) {
        
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }
        
        for (auto iter = queue.rbegin() + compcol.start_index; iter != queue.rbegin() + compcol.start_index + compcol.size; iter++) {
            PROFILE("CONSTRUCT ENTITY");
            auto comp = *iter;
            EntityParseResult entity_template = EntityManager::Get()->GetEntitySignature(comp.path);
            Entity new_ent = comp.id;
            world.CreateEntityFromEmpty(new_ent);
            script_vm->SetEngineInitializationEntity(new_ent,comp.path);
            world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
            script_vm->CallInitializationFunction(comp.path, "OnConstruct");

            for (auto child : entity_template.children) {
                EntityParseResult child_entity_template = EntityManager::Get()->GetEntitySignature(child);
                Entity child_ent = world.CreateEntity();
                script_vm->SetEngineInitializationEntity(child_ent, child);
                world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
                script_vm->CallInitializationFunction(child, "OnConstruct");
            }

        }
    };

    RunSystemSimpleQueue(world,EntityManager::Get()->GetQueue() ,func_1);
    EntityManager::Get()->ClearConstructionQueue();
}
