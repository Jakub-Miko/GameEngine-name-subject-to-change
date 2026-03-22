#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

class GenerateColorBufferPass : public RenderPass {
public:
    GenerateColorBufferPass(const std::string& output_color_framebuffer);

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;

    ~GenerateColorBufferPass() override = default;
private:

    void GenerateColorBuffer();

    std::string output_color_framebuffer;
    std::shared_ptr<RenderFrameBufferResource> output_color_buffer;
};
