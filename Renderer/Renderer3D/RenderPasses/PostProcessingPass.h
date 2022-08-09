#pragma once
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>



class PostProcessingPass : public RenderPass {
public:
	struct internal_data;
	PostProcessingPass(const std::string& input_buffer_name);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	~PostProcessingPass();
private:
	void InitPostProcessingPassData();
	std::string input_buffer_name;
	internal_data* data;
};