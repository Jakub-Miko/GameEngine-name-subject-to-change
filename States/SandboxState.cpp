#include "SandboxState.h"
#include <Application.h>


SandboxState::SandboxState() : GameState() {

}

void SandboxState::Update(float delta_time)
{
}

bool SandboxState::OnEvent(Event* e)
{
	return e->handled;
}

void SandboxState::OnAttach()
{
}

void SandboxState::OnDeattach()
{
}
