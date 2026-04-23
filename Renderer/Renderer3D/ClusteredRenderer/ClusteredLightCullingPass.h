#pragma once
#include "Renderer/Renderer3D/RenderPass.h"
#include "Renderer/Renderer3D/CommonRenderPasses/ShadowMappingPass.h"

#define CLUSTER_GRID_X 64
#define CLUSTER_GRID_Y 40
#define CLUSTER_GRID_Z 128

class Pipeline;

struct ClusteredLightLists {
    RUNTIME_TAG("ClusteredLightLists")
    std::shared_ptr<RenderBufferResource> cluster_buffer;
    std::shared_ptr<RenderBufferResource> light_assignment_buffer;
    std::shared_ptr<RenderBufferResource> point_light_buffer;
    std::shared_ptr<RenderBufferResource> directional_light_buffer;
    std::shared_ptr<RenderBufferResource> skylight_buffer;
    int num_of_point_lights = 0;
    int num_of_directional_lights = 0;
    int num_of_skylights = 0;
};

struct ClusteredPointLightData {
    glm::mat4 light_matrix;
    glm::vec4 position_and_radius;
    glm::vec4 Light_Color;
    float range;
    uint32_t shadow_index;
    float light_far_plane;
    uint8_t padding[4];
};

struct ClusteredSkyLightData {
    glm::vec4 Light_Color;
    uint32_t specular_map_index;
    uint32_t diffuse_map_index;
    uint8_t padding[8];
};

struct ClusteredDirectionalLightData {
    glm::mat4 light_matrix[HARD_CODE_CASCADES];
    glm::vec4 direction;
    glm::vec4 Light_Color;
    glm::vec2 shadowmap_pixel_size;
    uint32_t shadow_index;
    float light_far_plane;
    float shadow_bias;
    uint8_t padding[12];
};

class ClusteredLightCullingPass : public RenderPass {
public:
    explicit ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& input_shadowed_point_light_list_name,
        const std::string& input_shadowed_directional_light_list_name, const std::string& input_directional_shadow_cascades,
        const std::string& output_clustered_light_lists_name, const std::string& active_cluster_list);

    ~ClusteredLightCullingPass() override = default;

    void Setup(RenderPassResourceDefinnition& setup_builder) override;
    void Render(RenderPipelineResourceManager& resource_manager) override;

private:
    struct internal_data;

    void InitPass();

    void UpdatePipeline(bool force = false);
    void RebuildClusterGrid();

    std::string active_cluster_list;
    std::string input_global_light_list_name;
    std::string input_shadowed_point_light_list_name;
    std::string input_shadowed_directional_light_list_name;
    std::string input_directional_shadow_cascades;
    std::string output_clustered_light_lists_name;
    std::shared_ptr<DynamicProperty<bool>> frustum_culling;
    std::shared_ptr<DynamicProperty<bool>> frustum_culling_reduction;
    std::shared_ptr<DynamicProperty<bool>> box_culling;
    std::shared_ptr<DynamicProperty<bool>> cluster_per_warp;
    std::shared_ptr<DynamicProperty<DynamicPropertyAction>> update_pipeline;
    std::shared_ptr<DynamicProperty<glm::uvec3>> cluster_grid_resolution;
    glm::uvec3 current_cluster_grid_resolution;
    std::unique_ptr<internal_data> data;
};
