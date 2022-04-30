#include "ApplicationDataModule.h"
#include <Window.h>
#include <World/World.h>
#include <World/ScriptModules/MathModule.h>
#include <Application.h>
#include <glm/glm.hpp>
#include <iostream>

extern "C" {
    typedef struct entity { uint32_t id; } entity;

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
    MathModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
            typedef struct entity { uint32_t id; } entity;
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
        {"struct vec2", "vec2"}
        });

}



