#pragma once
#include <Renderer/Renderer3D/RenderPass.h>

class RenderSubmissionPass : public RenderPass {
public:

	RenderSubmissionPass(const std::string& output_collection_name = "RenderObjects");

	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;

	virtual void Render(RenderPipelineResourceManager& resource_manager) override;

private:
	std::string output_collection_name;
};
