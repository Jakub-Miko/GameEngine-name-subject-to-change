#include "RenderPipeline.h"

void RenderPipeline::Render()
{
	for (auto pass : passes) {
		RenderPipelineResourceManager manager(this, &pass);
		pass.render_pass->Render(manager);
	}
}

RenderPipeline::RenderPipeline(std::vector<RenderPassData>&& render_passes, std::unordered_map<std::string, RenderPipelineResourceData_internal>&& resources, 
	std::unordered_map<std::string, RenderPipelineResourceData_internal>&& persistent_resources) : passes(std::move(render_passes)) , resources(std::move(resources)), persistent_resources(std::move(persistent_resources))
{


}

RenderPipelineResourceManager::RenderPipelineResourceManager(RenderPipeline* pipeline, RenderPassData* current_pass) : pipeline(pipeline), current_pass(current_pass)
{

}
