#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <glm/glm.hpp>
#include <string>

class RenderCommandList;
class CameraComponent;

class DefferedLightingPass : public RenderPass {
public:
	struct internal_data;
	DefferedLightingPass(const std::string& input_gbuffer, const std::string& input_lights, const std::string& input_shadowed_lights, const std::string& output_buffer, const std::string& shadow_map_dependency_tag);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~DefferedLightingPass();
private:

	struct render_props {
		glm::mat4 projection;
		glm::mat4 view;
		float depth_constant_a;
		float depth_constant_b;
	};

	void RenderLights(RenderPipelineResourceManager& resource_manager, RenderCommandList* list, const CameraComponent& camera, const render_props& props);
	void RenderShadowedLights(RenderPipelineResourceManager& resource_manager, RenderCommandList* list, const CameraComponent& camera, const render_props& props);

	void InitPostProcessingPassData();
	std::string input_gbuffer;
	std::string input_lights;
	std::string input_shadowed_lights;
	std::string output_buffer;
	std::string shadow_map_dependency_tag;
	internal_data* data;
};