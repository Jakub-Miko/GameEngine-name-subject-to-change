#include "EntityConstructionSystem.h"
#include <World/System.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <FileManager.h>
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

            EntityManager::Get()->DeserializeEntityPrefab(*iter, comp.path, comp.parent);

            //EntityParseResult entity_template = EntityManager::Get()->GetEntitySignature(comp.path);
            //Entity new_ent = *iter;
            //
            ////For Deserialized entities
            //if (world.HasComponentSynced<LoadedComponent>(new_ent)) {
            //    script_vm->SetEngineInitializationEntity(new_ent, comp.path);
            //    script_vm->CallInitializationFunction(comp.path, "OnConstruct");
            //    world.SetComponent<InitializationComponent>(new_ent);
            //}
            //else { // for spawned entities
            //    world.SetComponent<LoadedComponent>(new_ent, LoadedComponent(comp.path));
            //    world.CreateEntityFromEmpty(new_ent, comp.parent);
            //    script_vm->SetEngineInitializationEntity(new_ent,comp.path);
            //    world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
            //    script_vm->CallInitializationFunction(comp.path, "OnConstruct");
            //    world.SetComponent<InitializationComponent>(new_ent);
            //}

            //if (!entity_template.component_json.empty()) {
            //    EntityManager::Get()->DeserializeComponents(new_ent, entity_template.component_json);
            //}

            //for (auto child : entity_template.children) {
            //    auto path = FileManager::Get()->GetPath(child);
            //    
            //    EntityParseResult child_entity_template = EntityManager::Get()->GetEntitySignature(path);
            //    Entity child_ent = world.CreateEntity(new_ent);
            //    script_vm->SetEngineInitializationEntity(child_ent, path);
            //    world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
            //    script_vm->CallInitializationFunction(path, "OnConstruct");
            //    world.SetComponent<InitializationComponent>(child_ent);

            //    if (!child_entity_template.component_json.empty()) {
            //        EntityManager::Get()->DeserializeComponents(child_ent, child_entity_template.component_json);
            //    }

            //}

        }
    };

    RunSystemSimple<ConstructionComponent>(world, func_1);
    world.GetRegistry().clear<ConstructionComponent>();
}
