#include "FrameManager.h"
#include <ConfigManager.h>
#include <Profiler.h>

FrameManager* FrameManager::instance = nullptr;


void FrameManager::Initialize()
{
	if (!instance) {
		instance = new FrameManager();
	}
}

FrameManager* FrameManager::Get()
{
	return instance;
}

void FrameManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

void FrameManager::StartFrame()
{

	PROFILE("StartFrame Sync");
	++frame_number;
	Renderer::Get()->GetCommandQueue()->Signal(m_Sync_Fence, frame_number - 1);
	m_Sync_Fence->WaitForValue(frame_number - latency);

}


FrameManager::FrameManager()
{
	frame_number = ConfigManager::Get()->GetInt("Latency_Frames");
	latency = frame_number;
	m_Sync_Fence.reset(Renderer::Get()->GetFence());
}

FrameManager::~FrameManager()
{
}
