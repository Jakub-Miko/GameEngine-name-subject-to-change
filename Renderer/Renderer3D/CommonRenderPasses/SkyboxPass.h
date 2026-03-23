#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

class SkyboxPass : public RenderPass {
public:
    SkyboxPass(const std::string& input_color_buffer, const std::string& input_clustered_lights,  const std::string& output_color_buffer);

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;
    ~SkyboxPass() override;

private:
    struct internal_data;
    internal_data* data;

    void InitSkyboxPassData();

    void UpdateFramebuffer(std::shared_ptr<RenderFrameBufferResource> input_buffer);

    std::string input_color_buffer;
    std::string input_clustered_lights;
    std::string output_color_buffer;
};
