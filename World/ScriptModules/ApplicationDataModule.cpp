#include "ApplicationDataModule.h"
#include <Window.h>
#include <World/World.h>
#include <Application.h>
#include <glm/glm.hpp>

static glm::vec2 GetWindowResolution() {
    auto& props = Application::Get()->GetWindow()->GetProperties();
    return glm::vec2(props.resolution_x, props.resolution_y);
}

static void SetPrimaryEntity(Entity ent) {
    Application::GetWorld().SetPrimaryEntity(ent);
}

static Entity GetPrimaryEntity() {
    return Application::GetWorld().GetPrimaryEntity();
}

void ApplicationDataModule::RegisterModule(std::vector<LuaEngine::LuaEngine_Function_Binding>& binding_list)
{
    binding_list.insert(binding_list.end(), {
        //This is where function bindings go
        LUA_FUNCTION("GetWindowResolution",GetWindowResolution),
        LUA_FUNCTION("SetPrimaryEntity",SetPrimaryEntity),
        LUA_FUNCTION("GetPrimaryEntity",GetPrimaryEntity)
        });
}



