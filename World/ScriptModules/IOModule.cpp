#include "IOModule.h"
#include <glm/glm.hpp>
#include <Input/Input.h>
#include <World/Components/ScriptComponent.h>
#include <World/ScriptModules/MathModule.h>

extern "C" {
    vec2 GetMousePosition_L()
    {
        return *reinterpret_cast<vec2*>(&Input::Get()->GetMoutePosition());
    }

    bool IsKeyPressed_L(int key_code)
    {
        return Input::Get()->IsKeyPressed((KeyCode)key_code);
    }

    bool IsMouseButtonPressed_L(int key_code)
    {
        return Input::Get()->IsMouseButtonPressed((MouseButtonCode)key_code);
    }
}

void IOModule::OnRegisterModule(ModuleBindingProperties& props)
{
    MathModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
    vec2 GetMousePosition_L();
    bool IsKeyPressed_L(int key_code);
    bool IsMouseButtonPressed_L(int key_code);
    )");

    props.Add_FFI_aliases({
        {"GetMousePosition_L","GetMousePosition"},
        {"IsKeyPressed_L","IsKeyPressed"},
        {"IsMouseButtonPressed_L","IsMouseButtonPressed_"},
        });

}



