#include "SandboxState.h"
#include <Application.h>

void SandboxState::Update(float delta_time)
{
	GetGameLayer()->OnUpdate(delta_time);
}

bool SandboxState::OnEvent(Event* e)
{
	GetGameLayer()->OnEvent(e);
	return e->handled;
}

void SandboxState::OnAttach()
{

}

void SandboxState::OnDeattach()
{

}
