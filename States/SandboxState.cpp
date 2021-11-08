#include "SandboxState.h"
#include <Application.h>
#include <TestLayer.h>

void SandboxState::Update(float delta_time)
{
	m_TestLayer->OnUpdate(delta_time);
	GetGameLayer()->OnUpdate(delta_time);
}

bool SandboxState::OnEvent(Event* e)
{
	m_TestLayer->OnEvent(e);
	GetGameLayer()->OnEvent(e);
	return e->handled;
}

void SandboxState::OnAttach()
{
	m_TestLayer = new TestLayer();
}

void SandboxState::OnDeattach()
{
	delete m_TestLayer;
}
