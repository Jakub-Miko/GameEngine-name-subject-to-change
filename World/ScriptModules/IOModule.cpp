#include "IOModule.h"
#include <glm/glm.hpp>
#include <Input/Input.h>
#include <World/Components/ScriptComponent.h>
#include <World/ScriptModules/MathModule.h>
#include <Core/Defines.h>

extern "C" {
    LIBEXP vec2 GetMousePosition_L()
    {
        auto pos = Input::Get()->GetMousePosition();
        return vec2{ pos.x,pos.y };
    }

    LIBEXP vec2 GetMousePositionChange_L()
    {
        auto pos = Input::Get()->GetMousePositionChange();
        return vec2{ pos.x,pos.y };
    }

    LIBEXP bool IsKeyPressed_L(int key_code)
    {
        return Input::Get()->IsKeyPressed((KeyCode)key_code);
    }

    LIBEXP bool IsMouseButtonPressed_L(int key_code)
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
    vec2 GetMousePositionChange_L();
    )");

    props.Add_FFI_aliases({
        {"GetMousePosition_L","GetMousePosition"},
        {"IsKeyPressed_L","IsKeyPressed"},
        {"IsMouseButtonPressed_L","IsMouseButtonPressed"},
        {"GetMousePositionChange_L","GetMousePositionChange"}
        });

}



