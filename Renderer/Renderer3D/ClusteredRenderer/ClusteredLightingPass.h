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
		const std::string& input_point_shadowed_lights, const std::string& output_texture, const std::string& shadow_map_dependency_tag, const std::string& input_directional_shadowed_cascades,
		const std::string& input_point_shadow_maps, const std::string& input_directional_shadow_maps);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;

	virtual ~ClusteredLightingPass();
private:

	struct render_props {
		glm::mat4 projection;
		glm::mat4 view;
		float depth_constant_a;
		float depth_constant_b;
	};

	struct ClusteredPipelineConfig {
		std::shared_ptr<DynamicProperty<bool>> use_compute_for_clustered_lights;
		std::shared_ptr<DynamicProperty<bool>> scalarize_lights;
		std::shared_ptr<DynamicProperty<uint32_t>> compute_tile_size;
		std::shared_ptr<DynamicProperty<DynamicPropertyAction>> needs_update;
	};

	void UpdateClusteredPipeline(bool force_update = false);

	void RenderLights(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);
	void RenderLightsWithCompute(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);

	void RenderShadowedLightsDirectional(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);
	void RenderSkylights(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props);

	void InitPassData();
	std::string input_gbuffer;
	std::string input_gbuffer_material;
	std::string input_clustered_lights;
	std::string input_directional_shadowed_lights;
	std::string input_directional_shadowed_cascades;
	std::string input_point_shadowed_lights;
	std::string output_texture;
	std::string shadow_map_dependency_tag;
	std::string input_point_shadow_maps;
	std::string input_directional_shadow_maps;
	ClusteredPipelineConfig clustered_config;
	bool use_compute_for_clustered_lights = false;
	float compute_tile_size = 16;
	std::unique_ptr<EventObserverBase> clustered_config_observer;
	internal_data* data;
};