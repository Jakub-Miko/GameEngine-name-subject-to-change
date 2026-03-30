#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

class DebugOverlayPass : public RenderPass {
public:
    DebugOverlayPass(const std::string& input_gbuffer, const std::string& input_gbuffer_material,
        const std::string& input_clustered_lights, const std::string& input_light_accum_buffer, const std::string& output_overlay);
    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;
    ~DebugOverlayPass() override;


private:
    struct internal_data;
    internal_data* data;

    void InitPass();

    std::string input_gbuffer;
    std::string input_gbuffer_material;
    std::string input_clustered_lights;
    std::string input_light_accum_buffer;
    std::string output_overlay;
    std::shared_ptr<DynamicProperty<float>> overlay_opacity_prop;
    std::shared_ptr<DynamicProperty<bool>> enable_debug_overlay_prop;
    std::shared_ptr<DynamicProperty<MultiChoice>> debug_layer_mode_prop;
    std::shared_ptr<DynamicProperty<glm::uvec3>> cluster_grid_resolution;
};
