#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>
#include "Renderer/MaterialManager.h"


class DepthPrepass : public RenderPass {
public:
	struct internal_data;
	DepthPrepass(const std::string& input_geometry, const std::string& input_skeletal_geometry, const std::string& input_buffer, const std::string& output_buffer_after_prepass, const std::string& output_depth_buffer);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~DepthPrepass();
private:
	void InitPostProcessingPassData();


	struct render_props {
		explicit render_props(RenderPipelineResourceManager& resource_manager) : resource_manager(resource_manager) {}
		std::shared_ptr<RenderFrameBufferResource> gbuffer;
		glm::mat4 view_matrix = glm::mat4(1.0f);
		glm::mat4 projection_matrix = glm::mat4(1.0f);
		RenderPipelineResourceManager& resource_manager;
	};

	void RenderGeometry(std::shared_ptr<RenderCommandList> list, render_props& props);
	void RenderSkeletalGeometry(std::shared_ptr<RenderCommandList> list, render_props& props);

	void UpdatePrepassFrameBuffer(std::shared_ptr<RenderFrameBufferResource> g_buffer);

	std::string input_geometry;
	std::string input_skeletal_geometry;
	std::string output_buffer_after_prepass;
	std::string output_depth_buffer;
	std::string input_buffer;
	std::shared_ptr<DynamicProperty<bool>> enable_depth_prepass_prop = nullptr;
	internal_data* data;
};
