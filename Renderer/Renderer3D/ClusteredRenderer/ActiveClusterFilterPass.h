#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

class ActiveClusterFilterPass : public RenderPass {
public:
    explicit ActiveClusterFilterPass(const std::string& input_gbuffer_material,const std::string& input_gbuffer, const std::string& output_active_clusters);

    ~ActiveClusterFilterPass() override = default;

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;

private:
    struct internal_data;

    void InitPass();

    std::string input_gbuffer;
    std::string input_gbuffer_material;
    std::string output_active_clusters;
    std::unique_ptr<internal_data> data;
};
