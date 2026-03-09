#pragma once
#include "Renderer/Renderer3D/RenderPass.h"

#define CLUSTER_GRID_X 64
#define CLUSTER_GRID_Y 40
#define CLUSTER_GRID_Z 128

class Pipeline;

struct ClusteredLightLists {
    RUNTIME_TAG("ClusteredLightLists")
    std::shared_ptr<RenderBufferResource> cluster_buffer;
    std::shared_ptr<RenderBufferResource> light_assignment_buffer;
    std::shared_ptr<RenderBufferResource> light_buffer;
    int num_of_lights;
};

struct ClusteredLightData {
    glm::mat4 light_matrix;
    glm::vec4 position_or_direction_and_radius;
    glm::vec4 Light_Color;
    float range;
    int light_type;
    uint32_t shadow_index;
    float light_far_plane;
};

class ClusteredLightCullingPass : public RenderPass {
public:
    explicit ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& input_shadowed_light_list_name,  const std::string& output_clustered_light_lists_name, const std::string& active_cluster_list);

    ~ClusteredLightCullingPass() override = default;

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;

private:
    struct internal_data;

    void InitPass();
    std::shared_ptr<Pipeline> GetPipeline();

    std::string active_cluster_list;
    std::string input_global_light_list_name;
    std::string input_shadowed_light_list_name;
    std::string output_clustered_light_lists_name;
    std::unique_ptr<internal_data> data;
};
