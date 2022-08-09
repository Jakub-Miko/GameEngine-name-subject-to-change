#pragma once 
#include <Renderer/Renderer3D/RenderPass.h>
#include <string>


class DefferedGeometryPass : public RenderPass {
public:
	struct internal_data;
	DefferedGeometryPass(const std::string& input_geometry, const std::string& output_buffer);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	virtual ~DefferedGeometryPass();
private:
	void InitPostProcessingPassData();
	std::string input_geometry;
	std::string output_buffer;
	internal_data* data;
};