#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

#define CLUSTER_GRID_X 16
#define CLUSTER_GRID_Y 16
#define CLUSTER_GRID_Z 16

struct ClusteredLightLists {
    RUNTIME_TAG("ClusteredLightLists")
    std::shared_ptr<RenderBufferResource> cluster_buffer;
    std::shared_ptr<RenderBufferResource> light_assignment_buffer;
    std::shared_ptr<RenderBufferResource> light_buffer;
    int num_of_lights;
};

struct ClusteredLightData {
    glm::vec4 position_or_direction_and_radius;
    glm::vec4 Light_Color;
    glm::vec4 attenuation_constants;
    int light_type;
    uint8_t padding[12];
};

class ClusteredLightCullingPass : public RenderPass {
public:
    explicit ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& output_clustered_light_lists_name, const std::string& active_cluster_list);

    ~ClusteredLightCullingPass() override = default;

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;

private:
    struct internal_data;

    void InitPass();

    std::string active_cluster_list;
    std::string input_global_light_list_name;
    std::string output_clustered_light_lists_name;
    std::unique_ptr<internal_data> data;
};
