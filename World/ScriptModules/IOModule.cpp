#include "IOModule.h"
#include <glm/glm.hpp>
#include <Input/Input.h>
#include <World/Components/ScriptComponent.h>

static glm::vec2 GetMousePosition()
{
    return Input::Get()->GetMoutePosition();
}

static bool IsKeyPressed(int key_code)
{
    return Input::Get()->IsKeyPressed((KeyCode)key_code);
}

static bool IsMouseButtonPressed(int key_code)
{
    return Input::Get()->IsMouseButtonPressed((MouseButtonCode)key_code);
}

void IOModule::RegisterModule(std::vector<LuaEngine::LuaEngine_Function_Binding>& binding_list)
{
    binding_list.insert(binding_list.end(), {
        //This is where function bindings go
        LUA_FUNCTION("IsKeyPressed", IsKeyPressed),
        LUA_FUNCTION("IsMouseButtonPressed", IsMouseButtonPressed),
        LUA_FUNCTION("GetMousePosition", GetMousePosition)
        });
}



