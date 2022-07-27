#pragma once 
#include "RenderPass.h"

class RenderPipeline {
public:

	void Render();

private:
	friend class RenderPassBuilder;
	RenderPipeline(std::vector<std::shared_ptr<RenderPass>>&& render_passes);
	std::vector<std::shared_ptr<RenderPass>> passes;

};