#pragma once 
#include <stdint.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderFence.h>

class FrameManager {
public:

	static void Initialize();
	static FrameManager* Get();
	static void Shutdown();

public:

	void StartFrame();
	uint32_t GetCurrentFrameNumber() const {
		return frame_number;
	}

	uint32_t GetRendererFrameNumber() const {
		return m_Sync_Fence->GetValue();
	}

	uint32_t GetLatencyFrames() const {
		return latency;
	}

private:
	static FrameManager* instance;
	FrameManager();
	~FrameManager();

private:
	uint32_t frame_number = 0;
	uint32_t latency = 0;
	std::shared_ptr<RenderFence> m_Sync_Fence;

};