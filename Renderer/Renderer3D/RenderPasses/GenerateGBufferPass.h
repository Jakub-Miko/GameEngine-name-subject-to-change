#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>


class GenerateGBufferPass : public RenderPass {
public:
	struct internal_data;
	GenerateGBufferPass(const std::string& output_buffer);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~GenerateGBufferPass();
private:
	void InitPostProcessingPassData();
	std::string output_buffer;
	internal_data* data;
};