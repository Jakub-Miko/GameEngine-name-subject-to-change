#include "ApplicationDataModule.h"
#include <Window.h>
#include <World/World.h>
#include <World/ScriptModules/MathModule.h>
#include <Application.h>
#include <glm/glm.hpp>
#include <iostream>
#include "GlobalEntityModule.h"

extern "C" {

    void print_s(const char* string) {
        std::cout << "FFI says: " << string << "\n";
    }

    vec2 GetWindowResolution_L() {
        auto& props = Application::Get()->GetWindow()->GetProperties();
        return vec2{ (float)props.resolution_x, (float)props.resolution_y };
    }

    void SetPrimaryEntity_L(entity ent) {
        Application::GetWorld().SetPrimaryEntity(ent.id);
    }

    entity GetPrimaryEntity_L() {
        return entity{Application::GetWorld().GetPrimaryEntity().id};
    }

}

void ApplicationDataModule::OnRegisterModule(ModuleBindingProperties& props)
{
    GlobalEntityModule().RegisterModule(props);
    MathModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
            vec2 GetWindowResolution_L();
            void SetPrimaryEntity_L(entity ent);
            void print_s(const char* string);
            entity GetPrimaryEntity_L();
    )");

    props.Add_FFI_aliases({
        {"GetWindowResolution_L","GetWindowResolution"},
        {"print_s","print_s"},
        {"GetPrimaryEntity_L","GetPrimaryEntity"},
        {"SetPrimaryEntity_L","SetPrimaryEntity"},
        });

}



