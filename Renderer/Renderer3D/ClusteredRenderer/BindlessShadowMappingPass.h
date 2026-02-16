#pragma once
#include <Renderer/Renderer3D/RenderPass.h>
#include <World/Components/LightComponent.h>
#include <World/Entity.h>
#include <string>

class RenderCommandList;

class BindlessShadowMappingPass : public RenderPass {
public:
	struct internal_data;
	BindlessShadowMappingPass(const std::string& input_shadow_casters_directional, const std::string& input_shadow_casters_point, const std::string& output_dependency_tag, const std::string& output_shadow_cascades
		,const std::string& cascaded_shadowmap_store, const std::string& cubemap_shadowmap_store);
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) override;
	virtual void Render(RenderPipelineResourceManager& resource_manager) override;
	~BindlessShadowMappingPass();
private:
	void InitShadowComponent(Entity ent, LightType light_type, std::shared_ptr<RenderCommandList> list);
	
	template<LightType type>
	void Render_impl(RenderPipelineResourceManager& resource_manager);

	void RenderDirectionalShadowCaster(Entity caster, std::shared_ptr<RenderCommandList>  list);
	
	void RenderPointShadowCaster(Entity caster, std::shared_ptr<RenderCommandList>  list);

	void InitShadowMappingPassData();
	std::string input_shadow_casters_directional;
	std::string input_shadow_casters_point;
	std::string output_dependency_tag;
	std::string output_shadow_cascades;
	std::string cascaded_shadowmap_store;
	std::string cubemap_shadowmap_store;
	internal_data* data;
};