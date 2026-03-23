#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>
#include "Renderer/MaterialManager.h"


class ClusteredForwardPass : public RenderPass {
public:
	struct internal_data;
	ClusteredForwardPass(const std::string& input_geometry, const std::string& input_color_buffer, const std::string& input_skeletal_geometry, const std::string& input_clustered_lights, const std::string& input_directional_shadowed_lights,
		const std::string& input_point_shadowed_lights, const std::string& output_buffer, const std::string& shadow_map_dependency_tag,
		const std::string& input_point_shadow_maps, const std::string& input_directional_shadow_maps);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~ClusteredForwardPass();
private:
	void InitPostProcessingPassData();


	struct render_props {
		explicit render_props(RenderPipelineResourceManager& resource_manager) : resource_manager(resource_manager) {}
		glm::mat4 view_matrix = glm::mat4(1.0f);
		std::shared_ptr<Material> default_material;
		std::shared_ptr<RenderFrameBufferResource> output_buffer;
		RenderPipelineResourceManager& resource_manager;
	};

	void RenderGeometry(std::shared_ptr<RenderCommandList> list, render_props& props);
	void RenderSkeletalGeometry(std::shared_ptr<RenderCommandList> list, render_props& props);

	std::string input_geometry;
	std::string input_skeletal_geometry;
	std::string input_clustered_lights;
	std::string input_directional_shadowed_lights;
	std::string input_point_shadowed_lights;
	std::string output_buffer;
	std::string shadow_map_dependency_tag;
	std::string input_point_shadow_maps;
	std::string input_directional_shadow_maps;
	std::string input_color_buffer;
	internal_data* data;
};
