#include "SandboxState.h"
#include <Application.h>
#include <TestLayer.h>
#include <FileManager.h>

#include "TestLayers/OneSweepRadixSort.h"

SandboxState::SandboxState() : GameState(), m_TestLayer(nullptr) {

}

void SandboxState::Update(float delta_time)
{
	m_TestLayer->OnUpdate(delta_time);
}

bool SandboxState::OnEvent(Event* e)
{
	m_TestLayer->OnEvent(e);
	return e->handled;
}

void SandboxState::OnAttach()
{
	m_TestLayer = new OneSweepRadixSort();
}

void SandboxState::OnDeattach()
{
	delete m_TestLayer;
}
