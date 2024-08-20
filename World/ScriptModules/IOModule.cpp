#include "IOModule.h"
#include <glm/glm.hpp>
#include <Input/Input.h>
#include <World/Components/ScriptComponent.h>
#include <World/ScriptModules/MathModule.h>
#include <Core/Defines.h>

extern "C" {

    /**
     * @brief Gets the current mouse position on the screen 
     * @return vec2 containing the position of the mouse on the screen in NDC(Normalized Device Coordinates, from -1 to 1) 
     * @lua
     */
    LIBEXP vec2 GetMousePosition_L()
    {
        auto pos = Input::Get()->GetMousePosition();
        return vec2{ pos.x,pos.y };
    }

    /**
     * @brief Returns the change in the mouse position over the last two frames
     * @return the change in mouse position in NDC(Normalized Device Coordinates, from -1 to 1) 
     * @lua
     */
    LIBEXP vec2 GetMousePositionChange_L()
    {
        auto pos = Input::Get()->GetMousePositionChange();
        return vec2{ pos.x,pos.y };
    }

    /**
     * @brief Returns the press state of a key 
     * @param key_code The keycode of the key state to poll defined by @ref KeyCode
     * @return whether the key is currently pressed
     * @lua
     */
    LIBEXP bool IsKeyPressed_L(int key_code)
    {
        return Input::Get()->IsKeyPressed((KeyCode)key_code);
    }

    /**
     * @brief Returns the press state of a mouse button 
     * @param key_code The keycode of the key state to poll defined by @ref MouseButtonCode
     * @return whether the mouse button is currently pressed
     * @lua
     */
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



