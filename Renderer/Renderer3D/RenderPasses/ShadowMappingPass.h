#pragma once
#include <Renderer/Renderer3D/RenderPass.h>
#include <World/Entity.h>
#include <string>

class ShadowMappingPass : public RenderPass {
public:
	struct internal_data;
	ShadowMappingPass(const std::string& input_shadow_casters, const std::string& output_dependency_tag);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	~ShadowMappingPass();
private:
	void InitShadowComponent(Entity ent);
	void InitShadowMappingPassData();
	std::string input_shadow_casters;
	std::string output_dependency_tag;
	internal_data* data;
};