#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <glm/glm.hpp>
#include <string>
#include <World/Components/LightComponent.h>

class Pipeline;
class RenderCommandList;
class CameraComponent;

class ClusteredLightingPass : public RenderPass {
public:
	struct internal_data;
	ClusteredLightingPass(const std::string& input_gbuffer, const std::string& input_gbuffer_material, const std::string& input_clustered_lights, const std::string& input_directional_shadowed_lights,
		const std::string& input_point_shadowed_lights, const std::string& output_buffer, const std::string& shadow_map_dependency_tag, const std::string& input_directional_shadowed_cascades,
		const std::string& input_point_shadow_maps, const std::string& input_directional_shadow_maps);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;

	enum class OutputModes : unsigned char {
		NORMAL = 0,
		LIGHT_COUNT = 1,
		CLUSTERS = 2,
		DEPTH_SLICES = 3,
		RADIUS = 4,
		TILES = 5,
	};

	[[nodiscard]] OutputModes GetActiveOutputMode() const { return active_output_mode; }
	void SetActiveOutputMode(OutputModes mode) { active_output_mode = mode; }

	virtual ~ClusteredLightingPass();
private:

	struct render_props {
		glm::mat4 projection;
		glm::mat4 view;
		float depth_constant_a;
		float depth_constant_b;
	};

	std::shared_ptr<Pipeline> GetPipelineForMode(OutputModes mode);

	void RenderLights(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);
	

	void RenderShadowedLightsPoint(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);
	void RenderShadowedLightsDirectional(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);
	void RenderSkylights(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);

	void InitPassData();
	std::string input_gbuffer;
	std::string input_gbuffer_material;
	std::string input_clustered_lights;
	std::string input_directional_shadowed_lights;
	std::string input_directional_shadowed_cascades;
	std::string input_point_shadowed_lights;
	std::string output_buffer;
	std::string shadow_map_dependency_tag;
	std::string input_point_shadow_maps;
	std::string input_directional_shadow_maps;
	OutputModes active_output_mode = OutputModes::NORMAL;
	internal_data* data;
};