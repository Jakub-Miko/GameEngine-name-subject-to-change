#pragma once
#include <Renderer/Renderer3D/RenderPipeline.h>

class DeferredRenderingPipeline {
public:
	static std::shared_ptr<RenderPipeline> CreatePipeline();

};