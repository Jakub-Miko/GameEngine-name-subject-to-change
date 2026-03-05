#include "ClusteredLightCullingPass.h"

#include "Application.h"
#include "dependencies/OpenAL/fmt-11.1.4/include/fmt/base.h"
#include "Renderer/PipelineManager.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/Renderer3D/RenderResourceCollection.h"
#include "World/Components/CameraComponent.h"
#include "World/Components/LightComponent.h"
#include "World/Components/ShadowCasterComponent.h"

struct ClusteredLightCullingPass::internal_data {
    std::unordered_map<char, std::shared_ptr<Pipeline>> pipelines;
    std::shared_ptr<RenderBufferResource> config_buffer;
    std::shared_ptr<RenderBufferResource> allocator_buffer;
    ClusteredLightLists output_lists;
    bool cull_with_planes = true;
    bool cull_with_boxes = true;
    bool reduce_spheres = true;
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

ClusteredLightCullingPass::ClusteredLightCullingPass(const std::string& input_global_light_list_name, const std::string& input_shadowed_light_list_name,
    const std::string& output_clustered_light_lists_name, const std::string& active_cluster_list)
    : input_global_light_list_name(input_global_light_list_name), output_clustered_light_lists_name(output_clustered_light_lists_name),
    data(new internal_data), active_cluster_list(active_cluster_list), input_shadowed_light_list_name(input_shadowed_light_list_name)
{
    InitPass();
}

void ClusteredLightCullingPass::InitPass() {
    RenderBufferDescriptor buffer_desc(CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z * 2 * sizeof(uint32_t), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->output_lists.cluster_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 200000 * sizeof(uint32_t);
    data->output_lists.light_assignment_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 5000 * sizeof(glm::vec4);
    data->output_lists.light_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    RenderBufferDescriptor config_buffer_desc(sizeof(ConfigBufferStruct), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);
    data->config_buffer = RenderResourceManager::Get()->CreateBuffer(config_buffer_desc);

    RenderBufferDescriptor allocator_buffer_desc(sizeof(CullingData), RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->allocator_buffer = RenderResourceManager::Get()->CreateBuffer(allocator_buffer_desc);
}

std::shared_ptr<Pipeline> ClusteredLightCullingPass::GetPipeline() {
    char key = (char)data->cull_with_boxes | (char)data->cull_with_planes << 1 | (char)data->reduce_spheres << 2;
	auto fnd = data->pipelines.find(key);
	if(fnd != data->pipelines.end()) {
		return fnd->second;
	}

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

	ComputePipelineDescriptor pipeline_desc = {};
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredLightCullingShader.glsl", compiler_definitions);
	auto new_pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    data->pipelines[key] = new_pipeline;

	return new_pipeline;
}


void ClusteredLightCullingPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<RenderResourceCollection<Entity>>(input_global_light_list_name, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<RenderResourceCollection<Entity>>(input_shadowed_light_list_name, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<ClusteredLightLists>(output_clustered_light_lists_name, RenderPassResourceDescriptor_Access::WRITE);
    setup_builder.AddResource<std::shared_ptr<RenderBufferResource>>(active_cluster_list, RenderPassResourceDescriptor_Access::READ);
    setup_builder.GetProperties()->SetProperty("Cull lights with frustum planes", true);
    setup_builder.GetProperties()->SetProperty("Cull lights with bounding boxes", true);
    setup_builder.GetProperties()->SetProperty("Reduce culling spheres on intersecting planes", true);
}

void ClusteredLightCullingPass::Render(RenderPipelineResourceManager& resource_manager) {
    PROFILE("ClusteredLightCullingPass");
    RenderResourceCollection<Entity> global_light_list[] =  {
        resource_manager.GetResource<RenderResourceCollection<Entity>>(input_global_light_list_name),
        resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadowed_light_list_name)
    };
    auto active_clusters = resource_manager.GetResource<std::shared_ptr<RenderBufferResource>>(active_cluster_list);
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
    std::vector<ClusteredLightData> light_positions_and_radii;
    light_positions_and_radii.reserve(global_light_list[0].resources.size() + global_light_list[1].resources.size());

    auto inverse_view = Application::GetWorld().GetComponent<TransformComponent>(Application::GetWorld().GetPrimaryEntity()).TransformMatrix;

    for(auto& list : global_light_list) {
        for(auto entity : list.resources) {
            auto transform = glm::inverse(camera_trans.TransformMatrix) * world.GetComponent<TransformComponent>(entity).TransformMatrix;
            auto& light = world.GetComponent<LightComponent>(entity);
            ClusteredLightData light_data = {};
            light_data.attenuation_constants = glm::vec4(light.GetAttenuation(),1.0);
            light_data.Light_Color = light.GetLightColor();
            light_data.light_type = (int)light.type;
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

    list->SetPipeline(GetPipeline());
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
