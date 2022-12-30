#pragma once
#include <Renderer/Renderer3D/RenderPass.h>
#include <World/Components/LightComponent.h>
#include <World/Entity.h>
#include <string>

class RenderCommandList;

class ShadowMappingPass : public RenderPass {
public:
	struct internal_data;
	ShadowMappingPass(const std::string& input_shadow_casters_directional, const std::string& input_shadow_casters_point, const std::string& output_dependency_tag, const std::string& output_shadow_cascades);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	~ShadowMappingPass();
private:
	void InitShadowComponent(Entity ent, LightType light_type);
	
	template<LightType type>
	void Render_impl(RenderPipelineResourceManager& resource_manager);

	void RenderDirectionalShadowCaster(Entity caster, RenderCommandList* list);
	
	void RenderPointShadowCaster(Entity caster, RenderCommandList* list);

	void InitShadowMappingPassData();
	std::string input_shadow_casters_directional;
	std::string input_shadow_casters_point;
	std::string output_dependency_tag;
	std::string output_shadow_cascades;
	internal_data* data;
};