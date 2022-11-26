#pragma once
#include <Renderer/Renderer3D/RenderPass.h>

class RenderSubmissionPass : public RenderPass {
public:

	RenderSubmissionPass(const std::string& output_mesh_name = "RenderObjects", const std::string& output_skeletal_mesh_name = "SkeletalRenderObjects", const std::string& output_light_name = "RenderLights",
		const std::string& output_shadowed_directional_light_name = "RenderShadowedDirectionalLights", const std::string& output_shadowed_point_light_name = "RenderShadowedPointLights");

	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;

	virtual void Render(RenderPipelineResourceManager& resource_manager) override;

private:
	std::string output_mesh_name;
	std::string output_skeletal_mesh_name;
	std::string output_light_name;
	std::string output_shadowed_directional_light_name;
	std::string output_shadowed_point_light_name;
};
