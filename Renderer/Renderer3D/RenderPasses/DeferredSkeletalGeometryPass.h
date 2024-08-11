#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>


class DeferredSkeletalGeometryPass : public RenderPass {
public:
	struct internal_data;
	DeferredSkeletalGeometryPass(const std::string& input_geometry, const std::string& input_buffer, const std::string& output_buffer);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~DeferredSkeletalGeometryPass();
private:
	void InitPostProcessingPassData();
	std::string input_geometry;
	std::string output_buffer;
	std::string input_buffer;
	internal_data* data;
};