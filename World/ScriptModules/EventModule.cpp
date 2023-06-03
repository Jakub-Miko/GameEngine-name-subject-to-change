#include "EventModule.h"
#include <Window.h>
#include <World/World.h>
#include <Application.h>
#include <glm/glm.hpp>
#include <iostream>
#include <World/ScriptModules/LocalEntityModule.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <Core/Defines.h>

extern "C" {

    LIBEXP void EnableMouseButtonPressedEvents_L() {
        Entity ent = Entity(GetCurrentEntity_L().id);
        Application::GetWorld().SetComponent<MousePressedScriptComponent>(ent);
    }

    LIBEXP void EnableKeyPressedEvents_L() {
        Entity ent = Entity(GetCurrentEntity_L().id);
        Application::GetWorld().SetComponent<KeyPressedScriptComponent>(ent);
    }

}

void EventModule::OnRegisterModule(ModuleBindingProperties& props)
{
    LocalEntityModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
        void EnableKeyPressedEvents_L();
        void EnableMouseButtonPressedEvents_L();
    )");

    props.Add_FFI_aliases({
        {"EnableKeyPressedEvents_L","EnableKeyPressedEvents"},
        {"EnableMouseButtonPressedEvents_L","EnableMouseButtonPressedEvents"}
        });

}



