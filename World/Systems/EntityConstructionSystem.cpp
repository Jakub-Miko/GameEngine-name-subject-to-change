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

    auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
    if (!script_vm) {
        ScriptSystemManager::Get()->InitializeScriptSystemVM();
        script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
    }

    auto view = Application::GetWorld().GetRegistry().view<ConstructionComponent>();

    for (auto ent : view) {
        PROFILE("CONSTRUCT ENTITY");
        auto comp = world.GetComponent<ConstructionComponent>(ent);

        EntityManager::Get()->DeserializeEntityPrefab(ent, comp.path, comp.parent);


    }

    world.GetRegistry().clear<ConstructionComponent>();
}
