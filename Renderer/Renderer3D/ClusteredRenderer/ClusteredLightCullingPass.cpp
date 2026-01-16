#include "ClusteredLightCullingPass.h"

#include "Application.h"
#include "dependencies/OpenAL/fmt-11.1.4/include/fmt/base.h"
#include "Renderer/PipelineManager.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/Renderer3D/RenderResourceCollection.h"
#include "World/Components/CameraComponent.h"
#include "World/Components/LightComponent.h"

struct ClusteredLightCullingPass::internal_data {
    std::shared_ptr<Pipeline> culling_pipeline;
    std::shared_ptr<RenderBufferResource> config_buffer;
    std::shared_ptr<RenderBufferResource> allocator_buffer;
    ClusteredLightLists output_lists;
};

struct ConfigBufferStruct {
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;
    glm::uvec3 cluster_grid_size;
    uint32_t light_count;
    uint32_t light_assignment_size;
    float near_plane;
    float far_plane;
    float fov;
    float aspect_ratio;
};

struct CullingData {
    uint32_t light_allocation_index = 0;
    int32_t success_flag = 1;
};

ClusteredLightCullingPass::ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& output_clustered_light_lists_name, const std::string& active_cluster_list)
    : input_global_light_list_name(input_global_light_list_name), output_clustered_light_lists_name(output_clustered_light_lists_name), data(new internal_data), active_cluster_list(active_cluster_list)
{
    InitPass();
}

void ClusteredLightCullingPass::InitPass() {
    RenderBufferDescriptor buffer_desc(5000 * 2 * sizeof(uint32_t), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->output_lists.cluster_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 200000 * sizeof(uint32_t);
    data->output_lists.light_assignment_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 5000 * sizeof(glm::vec4);
    data->output_lists.light_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    RenderBufferDescriptor config_buffer_desc(sizeof(ConfigBufferStruct), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);
    data->config_buffer = RenderResourceManager::Get()->CreateBuffer(config_buffer_desc);

    RenderBufferDescriptor allocator_buffer_desc(sizeof(CullingData), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->allocator_buffer = RenderResourceManager::Get()->CreateBuffer(allocator_buffer_desc);

    ComputePipelineDescriptor pipeline_desc = {};
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredLightCullingShader.glsl");
    data->culling_pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);
}

void ClusteredLightCullingPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<RenderResourceCollection<Entity>>(input_global_light_list_name, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<ClusteredLightLists>(output_clustered_light_lists_name, RenderPassResourceDescriptor_Access::WRITE);
    setup_builder.AddResource<std::shared_ptr<RenderBufferResource>>(active_cluster_list, RenderPassResourceDescriptor_Access::READ);
}

void ClusteredLightCullingPass::Render(RenderPipelineResourceManager& resource_manager) {
    auto global_light_list = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_global_light_list_name);
    auto active_clusters = resource_manager.GetResource<std::shared_ptr<RenderBufferResource>>(active_cluster_list);
    if(global_light_list.resources.empty()) {
        ClusteredLightLists empty_lists = {};
        resource_manager.SetResource<ClusteredLightLists>(output_clustered_light_lists_name, empty_lists);
        return;
    };

    auto& world = Application::GetWorld();
    auto camera = world.GetPrimaryEntity();
    auto& camera_trans = world.GetComponent<TransformComponent>(camera);
    auto& camera_props = world.GetComponent<CameraComponent>(camera);
    auto list = Renderer::Get()->GetRenderCommandList();
    std::vector<ClusteredLightData> light_positions_and_radii;
    light_positions_and_radii.reserve(global_light_list.resources.size());

    for(auto entity : global_light_list.resources ) {
        auto transform = glm::inverse(camera_trans.TransformMatrix) * world.GetComponent<TransformComponent>(entity).TransformMatrix;
        auto& light = world.GetComponent<LightComponent>(entity);
        ClusteredLightData light_data = {};
        light_data.attenuation_constants = glm::vec4(light.GetAttenuation(),1.0);
        light_data.Light_Color = light.GetLightColor();
        light_data.light_type = (int)light.type;

        switch(light.type) {
        case LightType::DIRECTIONAL:
                light_data.position_or_direction_and_radius = glm::vec4(glm::mat3(transform) * glm::vec3(0.0f, 0.0f, -1.0f), 0.0f);
                break;
            case LightType::POINT:
                light_data.position_or_direction_and_radius = glm::vec4(glm::vec3(transform[3]), light.CalcRadiusFromAttenuation());
            default: break;
        }
        light_positions_and_radii.push_back(light_data);
    }

    CullingData culling_data = {};
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->allocator_buffer, &culling_data, sizeof(CullingData), 0);

    RenderResourceManager::Get()->UploadDataToBuffer(list, data->output_lists.light_buffer, light_positions_and_radii.data(), sizeof(ClusteredLightData) * light_positions_and_radii.size(), 0);

    ConfigBufferStruct config_buffer_struct = {};
    config_buffer_struct.projection_matrix = camera_props.GetProjectionMatrix();
    config_buffer_struct.view_matrix = glm::inverse(camera_trans.TransformMatrix);
    config_buffer_struct.light_count = static_cast<uint32_t>(light_positions_and_radii.size());
    config_buffer_struct.cluster_grid_size = glm::uvec3(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
    config_buffer_struct.near_plane = camera_props.zNear;
    config_buffer_struct.far_plane = camera_props.zFar;
    config_buffer_struct.fov = glm::radians(camera_props.fov);
    config_buffer_struct.aspect_ratio = camera_props.aspect_ratio;
    config_buffer_struct.light_assignment_size = static_cast<uint32_t>(data->output_lists.light_assignment_buffer->GetBufferDescriptor().buffer_size / sizeof(uint32_t));
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->config_buffer, &config_buffer_struct, sizeof(ConfigBufferStruct), 0);

    auto num_of_clusters = config_buffer_struct.cluster_grid_size.x * config_buffer_struct.cluster_grid_size.y * config_buffer_struct.cluster_grid_size.z;
    auto num_of_thread_groups = static_cast<int>(ceil(static_cast<double>(num_of_clusters) / 256.0));

    list->SetPipeline(data->culling_pipeline);
    list->SetConstantBuffer("config_buffer", data->config_buffer);
    list->SetStorageBuffer("light_buffer", data->output_lists.light_buffer);
    list->SetStorageBuffer("light_assignment_buffer", data->output_lists.light_assignment_buffer);
    list->SetStorageBuffer("cluster_buffer", data->output_lists.cluster_buffer);
    list->SetStorageBuffer("allocator_buffer", data->allocator_buffer);
    list->SetStorageBuffer("active_cluster_buffer", active_clusters);
    list->Dispatch(num_of_thread_groups, 1, 1);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);

    data->output_lists.num_of_lights = light_positions_and_radii.size();
    resource_manager.SetResource<ClusteredLightLists>(output_clustered_light_lists_name, data->output_lists);
}
