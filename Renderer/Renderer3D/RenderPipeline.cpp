#include "RenderPipeline.h"

void RenderPipeline::Render()
{
	for (auto pass : passes) {
		pass->Render();
	}
}

RenderPipeline::RenderPipeline(std::vector<std::shared_ptr<RenderPass>>&& render_passes) : passes(std::move(render_passes))
{

}
