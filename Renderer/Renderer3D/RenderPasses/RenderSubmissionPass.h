#pragma once
#include <Renderer/Renderer3D/RenderPass.h>

class RenderSubmissionPass : public RenderPass {
public:

	RenderSubmissionPass(const std::string& output_mesh_name = "RenderObjects", const std::string& output_light_name = "RenderLights", const std::string& output_shadowed_light_name = "RenderShadowedLights");

	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;

	virtual void Render(RenderPipelineResourceManager& resource_manager) override;

private:
	std::string output_mesh_name;
	std::string output_light_name;
	std::string output_shadowed_light_name;
};
