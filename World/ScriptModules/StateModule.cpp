#include "StateModule.h"
#include <glm/glm.hpp>
#include <Input/Input.h>
#include <World/Components/ScriptComponent.h>
#include <World/ScriptModules/MathModule.h>
#include <GameStateMachine.h>

extern "C" {
    void SetState_L(const char* state_name) {
        auto state = GameStateMachine::Get()->GetStateFromName(state_name);
        GameStateMachine::Get()->ChangeState(state);
    }


}

void StateModule::OnRegisterModule(ModuleBindingProperties& props)
{

    props.Add_FFI_declarations(R"(
    void SetState_L(const char* state_name);
    )");

    props.Add_FFI_aliases({
        {"SetState_L","SetState"}
        });

}



