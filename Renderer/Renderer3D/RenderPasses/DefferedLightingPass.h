#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>


class DefferedLightingPass : public RenderPass {
public:
	struct internal_data;
	DefferedLightingPass(const std::string& input_gbuffer, const std::string& input_lights, const std::string& output_buffer);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~DefferedLightingPass();
private:
	void InitPostProcessingPassData();
	std::string input_gbuffer;
	std::string input_lights;
	std::string output_buffer;
	internal_data* data;
};