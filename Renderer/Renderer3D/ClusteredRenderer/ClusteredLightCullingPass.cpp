#include "ClusteredLightCullingPass.h"

#include "Application.h"
#include "Renderer/PipelineManager.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/Renderer3D/RenderResourceCollection.h"
#include "World/Components/CameraComponent.h"
#include "World/Components/LightComponent.h"
#include "World/Components/ShadowCasterComponent.h"
#include "World/Components/SkylightComponent.h"

struct ClusteredLightCullingPass::internal_data {
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<RenderBufferResource> config_buffer;
    std::shared_ptr<RenderBufferResource> allocator_buffer;
    ClusteredLightLists output_lists;
    bool per_warp_optimization = false;
    bool cull_with_planes = true;
    bool cull_with_boxes = true;
    bool reduce_spheres = true;
};

struct ConfigBufferStruct {
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;
    glm::uvec3 cluster_grid_size;
    uint32_t point_light_count;
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

ClusteredLightCullingPass::ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& input_shadowed_point_light_list_name,
        const std::string& input_shadowed_directional_light_list_name, const std::string& input_directional_shadow_cascades,
        const std::string& output_clustered_light_lists_name, const std::string& active_cluster_list)
    : input_global_light_list_name(input_global_light_list_name), output_clustered_light_lists_name(output_clustered_light_lists_name),
    data(new internal_data), active_cluster_list(active_cluster_list), input_shadowed_point_light_list_name(input_shadowed_point_light_list_name),
    input_directional_shadow_cascades(input_directional_shadow_cascades), input_shadowed_directional_light_list_name(input_shadowed_directional_light_list_name)
{

}

void ClusteredLightCullingPass::InitPass() {
    auto cluster_grid_res = cluster_grid_resolution->GetValueTyped();
    UpdatePipeline(true);
    RenderBufferDescriptor buffer_desc(cluster_grid_res.x * cluster_grid_res.y * cluster_grid_res.z * 2 * sizeof(uint32_t), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->output_lists.cluster_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 30 * cluster_grid_res.x * cluster_grid_res.y * cluster_grid_res.z * sizeof(uint32_t);
    data->output_lists.light_assignment_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 5000 * sizeof(ClusteredPointLightData);
    data->output_lists.point_light_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 100 * sizeof(ClusteredDirectionalLightData);
    data->output_lists.directional_light_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 50 * sizeof(ClusteredSkyLightData);
    data->output_lists.skylight_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    RenderBufferDescriptor config_buffer_desc(sizeof(ConfigBufferStruct), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);
    data->config_buffer = RenderResourceManager::Get()->CreateBuffer(config_buffer_desc);

    RenderBufferDescriptor allocator_buffer_desc(sizeof(CullingData), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->allocator_buffer = RenderResourceManager::Get()->CreateBuffer(allocator_buffer_desc);
}

void ClusteredLightCullingPass::UpdatePipeline(bool force) {
    if(!update_pipeline->GetValueTyped().ShouldActivate() && !force) {
        return;
    }
    update_pipeline->GetValueTyped().Reset();
    data->cull_with_boxes = box_culling->GetValueTyped();
    data->cull_with_planes = frustum_culling->GetValueTyped();
    data->reduce_spheres = frustum_culling_reduction->GetValueTyped();
    data->per_warp_optimization = cluster_per_warp->GetValueTyped();

    std::vector<std::string> compiler_definitions;

    if(data->cull_with_planes) {
        compiler_definitions.emplace_back("CULL_WITH_PLANES");
    }

    if(data->cull_with_boxes) {
        compiler_definitions.emplace_back("CULL_WITH_BOXES");
    }

    if(data->reduce_spheres) {
        compiler_definitions.emplace_back("REDUCE_SPHERES_ON_INTERSECTING_PLANES");
    }

    if(data->per_warp_optimization) {
        compiler_definitions.emplace_back("CLUSTER_PER_WARP");
    }

    ComputePipelineDescriptor pipeline_desc = {};
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredLightCullingShader.glsl", compiler_definitions);
    data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);
}

void ClusteredLightCullingPass::RebuildClusterGrid() {
    auto cluster_grid_res = cluster_grid_resolution->GetValueTyped();
    if(current_cluster_grid_resolution == cluster_grid_res) {
        return;
    }
    RenderBufferDescriptor buffer_desc(cluster_grid_res.x * cluster_grid_res.y * cluster_grid_res.z * 2 * sizeof(uint32_t), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->output_lists.cluster_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 30 * cluster_grid_res.x * cluster_grid_res.y * cluster_grid_res.z * sizeof(uint32_t);
    data->output_lists.light_assignment_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
    current_cluster_grid_resolution = cluster_grid_res;
}


void ClusteredLightCullingPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<RenderResourceCollection<Entity>>(input_global_light_list_name, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<RenderResourceCollection<Entity>>(input_shadowed_point_light_list_name, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<RenderResourceCollection<Entity>>(input_shadowed_directional_light_list_name, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<ClusteredLightLists>(output_clustered_light_lists_name, RenderPassResourceDescriptor_Access::WRITE);
    setup_builder.AddResource<std::shared_ptr<RenderBufferResource>>(active_cluster_list, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<RenderResourceCollection<glm::mat4>>(input_directional_shadow_cascades, RenderPassResourceDescriptor_Access::READ);
    frustum_culling = setup_builder.GetProperties()->SetProperty("Cull lights with frustum planes", true).second;
    box_culling = setup_builder.GetProperties()->SetProperty("Cull lights with bounding boxes", true).second;
    frustum_culling_reduction = setup_builder.GetProperties()->SetProperty("Reduce culling spheres on intersecting planes", true).second;
    cluster_per_warp = setup_builder.GetProperties()->SetProperty("Clusters per Warp", true).second;
    update_pipeline = setup_builder.GetProperties()->SetProperty("Update culling pipeline", DynamicPropertyAction()).second;
    cluster_grid_resolution = setup_builder.GetProperties()->SetProperty("Cluster grid resolution", glm::uvec3(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z)).second;
    InitPass();
}

void ClusteredLightCullingPass::Render(RenderPipelineResourceManager& resource_manager) {
    PROFILE("ClusteredLightCullingPass");
    UpdatePipeline();
    RebuildClusterGrid();
    auto skylights = Application::GetWorld().GetRegistry().view<SkylightComponent>();
    RenderResourceCollection<Entity> global_light_list[] =  {
        resource_manager.GetResource<RenderResourceCollection<Entity>>(input_global_light_list_name),
        resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadowed_point_light_list_name),
        resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadowed_directional_light_list_name),
    };
    auto active_clusters = resource_manager.GetResource<std::shared_ptr<RenderBufferResource>>(active_cluster_list);
    auto& shadow_cascades = resource_manager.GetResource<RenderResourceCollection<glm::mat4>>(input_directional_shadow_cascades);
    if(global_light_list[0].resources.empty() && global_light_list[1].resources.empty()) {
        ClusteredLightLists empty_lists = {};
        resource_manager.SetResource<ClusteredLightLists>(output_clustered_light_lists_name, empty_lists);
        return;
    };

    auto dynamic_props = resource_manager.GetProperties();

    data->cull_with_planes = dynamic_props->GetProperty<bool>("Cull lights with frustum planes")->GetValueTyped();
    data->cull_with_boxes = dynamic_props->GetProperty<bool>("Cull lights with bounding boxes")->GetValueTyped();
    data->reduce_spheres = dynamic_props->GetProperty<bool>("Reduce culling spheres on intersecting planes")->GetValueTyped();

    auto& world = Application::GetWorld();
    auto camera = world.GetPrimaryEntity();
    auto& camera_trans = world.GetComponent<TransformComponent>(camera);
    auto& camera_props = world.GetComponent<CameraComponent>(camera);
    auto list = Renderer::Get()->GetRenderCommandList();
    std::vector<ClusteredPointLightData> point_lights;
    std::vector<ClusteredDirectionalLightData> directional_lights;
    std::vector<ClusteredSkyLightData> sky_lights;

    auto inverse_view = Application::GetWorld().GetComponent<TransformComponent>(Application::GetWorld().GetPrimaryEntity()).TransformMatrix;

    int cascade_counter = 0;
    for(auto& list : global_light_list) {
        for(auto entity : list.resources) {
            auto transform = glm::inverse(camera_trans.TransformMatrix) * world.GetComponent<TransformComponent>(entity).TransformMatrix;
            auto& light = world.GetComponent<LightComponent>(entity);
            switch(light.type) {
            case LightType::POINT:
            {
                ClusteredPointLightData light_data = {};
                light_data.range = light.GetLightRange();
                light_data.Light_Color = light.GetLightColor();
                if(world.HasComponent<ShadowCasterComponent>(entity)) {
                    auto& shadow = world.GetComponent<ShadowCasterComponent>(entity);
                    light_data.light_matrix = shadow.light_view_matrix * inverse_view;
                    if(shadow.shadow_map) {
                        light_data.shadow_index = shadow.shadow_map->GetBufferDescriptor().depth_stencil_attachment.resource->GetResourceStoreIndex();
                        light_data.light_far_plane = shadow.far_plane;
                    } else {
                        light_data.shadow_index = UINT32_MAX;
                    }
                } else {
                    light_data.shadow_index = UINT32_MAX;
                }
                light_data.position_and_radius = glm::vec4(glm::vec3(transform[3]), light.GetLightRange());
                point_lights.push_back(light_data);
                break;
            }
            case LightType::DIRECTIONAL:
            {
                ClusteredDirectionalLightData light_data = {};
                light_data.Light_Color = light.GetLightColor();
                if(world.HasComponent<ShadowCasterComponent>(entity)) {
                    auto& shadow = world.GetComponent<ShadowCasterComponent>(entity);
                    if(shadow.shadow_map) {
                        for (int i = 0; i < std::min(shadow.cascades,15); i++) {
                            light_data.light_matrix[i] = shadow_cascades.resources[cascade_counter] * inverse_view;
                            cascade_counter++;
                        }
                        light_data.shadow_index = shadow.shadow_map->GetBufferDescriptor().depth_stencil_attachment.resource->GetResourceStoreIndex();
                        light_data.light_far_plane = shadow.far_plane;
                        light_data.shadow_bias = shadow.shadow_bias;
                        light_data.shadowmap_pixel_size = glm::vec2(1.0f / (float)shadow.res_x, 1.0f / (float)shadow.res_y);
                    } else {
                        light_data.shadow_index = UINT32_MAX;
                    }
                } else {
                    light_data.shadow_index = UINT32_MAX;
                }
                light_data.direction = glm::vec4(glm::mat3(transform) * glm::vec3(0.0f, 0.0f, -1.0f), 0.0f);
                directional_lights.push_back(light_data);
                break;
            }
            default: break;
            }
        }
    }

    for(auto entity : skylights) {
        auto& light = world.GetComponent<SkylightComponent>(entity);
        if (!light.GetReflectionMap() || light.GetReflectionMap()->GetStatus() != ReflectionMapStatus::LOADED) {
            continue;
        }
        ClusteredSkyLightData light_data = {};
        light_data.Light_Color = light.GetLightColor();
        light_data.diffuse_map_index = light.GetReflectionMap()->GetDiffuseMap()->GetResourceStoreIndex();
        light_data.specular_map_index = light.GetReflectionMap()->GetSpecularMap()->GetResourceStoreIndex();
        sky_lights.push_back(light_data);
    }

    if(!point_lights.empty()) {
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->output_lists.point_light_buffer,
            point_lights.data(), sizeof(ClusteredPointLightData) * point_lights.size(), 0);
    }

    if(!directional_lights.empty()) {
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->output_lists.directional_light_buffer,
            directional_lights.data(), sizeof(ClusteredDirectionalLightData) * directional_lights.size(), 0);
    }

    if(!sky_lights.empty()) {
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->output_lists.skylight_buffer,
            sky_lights.data(), sizeof(ClusteredSkyLightData) * sky_lights.size(), 0);
    }

    CullingData culling_data = {};
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->allocator_buffer, &culling_data, sizeof(CullingData), 0);

    auto cluster_grid_res = cluster_grid_resolution->GetValueTyped();

    ConfigBufferStruct config_buffer_struct = {};
    config_buffer_struct.projection_matrix = camera_props.GetProjectionMatrix();
    config_buffer_struct.view_matrix = glm::inverse(camera_trans.TransformMatrix);
    config_buffer_struct.point_light_count = static_cast<uint32_t>(point_lights.size());
    config_buffer_struct.cluster_grid_size = glm::uvec3(cluster_grid_res.x, cluster_grid_res.y, cluster_grid_res.z);
    config_buffer_struct.near_plane = camera_props.zNear;
    config_buffer_struct.far_plane = camera_props.zFar;
    config_buffer_struct.fov = glm::radians(camera_props.fov);
    config_buffer_struct.aspect_ratio = camera_props.aspect_ratio;
    config_buffer_struct.light_assignment_size = static_cast<uint32_t>(data->output_lists.light_assignment_buffer->GetBufferDescriptor().buffer_size / sizeof(uint32_t));
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->config_buffer, &config_buffer_struct, sizeof(ConfigBufferStruct), 0);

    auto num_of_clusters = config_buffer_struct.cluster_grid_size.x * config_buffer_struct.cluster_grid_size.y * config_buffer_struct.cluster_grid_size.z;
    auto num_of_thread_groups = static_cast<int>(ceil(static_cast<double>(num_of_clusters) / 256.0));
    auto num_of_threads_with_per_warp_optimization = num_of_thread_groups * 32;

    list->SetPipeline(data->pipeline);
    list->SetConstantBuffer("config_buffer", data->config_buffer);
    list->SetStorageBuffer("light_buffer", data->output_lists.point_light_buffer);
    list->SetStorageBuffer("light_assignment_buffer", data->output_lists.light_assignment_buffer);
    list->SetStorageBuffer("cluster_buffer", data->output_lists.cluster_buffer);
    list->SetStorageBuffer("allocator_buffer", data->allocator_buffer);
    list->SetStorageBuffer("active_cluster_buffer", active_clusters);
    list->Dispatch(data->per_warp_optimization ? num_of_threads_with_per_warp_optimization : num_of_thread_groups, 1, 1);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);

    data->output_lists.num_of_point_lights = point_lights.size();
    data->output_lists.num_of_directional_lights = directional_lights.size();
    data->output_lists.num_of_skylights = sky_lights.size();
    resource_manager.SetResource<ClusteredLightLists>(output_clustered_light_lists_name, data->output_lists);
}
