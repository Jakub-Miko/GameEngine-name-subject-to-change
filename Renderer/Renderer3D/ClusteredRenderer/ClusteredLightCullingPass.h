#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

struct ClusteredLightLists {
    RUNTIME_TAG("ClusteredLightLists")
    std::shared_ptr<RenderBufferResource> cluster_buffer;
    std::shared_ptr<RenderBufferResource> light_assignment_buffer;
};

class ClusteredLightCullingPass : public RenderPass {
public:
    explicit ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& output_clustered_light_lists_name);

    ~ClusteredLightCullingPass() override = default;

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;

private:
    struct internal_data;

    void InitPass();

    std::string input_global_light_list_name;
    std::string output_clustered_light_lists_name;
    std::unique_ptr<internal_data> data;
};
