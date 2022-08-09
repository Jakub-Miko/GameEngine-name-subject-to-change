#pragma once
#include <Renderer/Renderer3D/RenderPipeline.h>

class DefferedRenderingPipeline {
public:
	static std::shared_ptr<RenderPipeline> CreatePipeline();

};