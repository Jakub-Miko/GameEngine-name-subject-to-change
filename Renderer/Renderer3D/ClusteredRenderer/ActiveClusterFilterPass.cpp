#include "ActiveClusterFilterPass.h"
#include "Application.h"
#include "Renderer/PipelineManager.h"
#include "ClusteredLightCullingPass.h"
#include "Window.h"
#include "Renderer/RenderResourceManager.h"
#include "World/Components/CameraComponent.h"
#include "World/Components/LightComponent.h"

struct ConfigBufferStruct {
    glm::uvec3 cluster_dimensions;
    float near_plane;
    glm::uvec2 window_size;
    float depth_constant_a;
    float depth_constant_b;
    float far_plane;
};

struct ActiveClusterFilterPass::internal_data {
    std::shared_ptr<RenderBufferResource> active_cluster_buffer;
    std::shared_ptr<RenderBufferResource> config_buffer;
    std::shared_ptr<Pipeline> active_cluster_filter_pipeline;
};

ActiveClusterFilterPass::ActiveClusterFilterPass(const std::string& input_depth_buffer, const std::string& output_active_clusters)
    : output_active_clusters(output_active_clusters), data(std::make_unique<internal_data>()), input_depth_buffer(input_depth_buffer)
{
    InitPass();
}

void ActiveClusterFilterPass::InitPass() {
    RenderBufferDescriptor active_cluster_buffer_desc((CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z + 1) * sizeof(uint32_t) , RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
    data->active_cluster_buffer = RenderResourceManager::Get()->CreateBuffer(active_cluster_buffer_desc);

    RenderBufferDescriptor config_buffer_desc(sizeof(ConfigBufferStruct), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);
    data->config_buffer = RenderResourceManager::Get()->CreateBuffer(config_buffer_desc);

    ComputePipelineDescriptor pipeline_desc = {};
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ActiveClusterFilterShader.glsl");
    data->active_cluster_filter_pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);
}

void ActiveClusterFilterPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<std::shared_ptr<RenderTexture2DResource>>(input_depth_buffer, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<std::shared_ptr<RenderBufferResource>>(output_active_clusters, RenderPassResourceDescriptor_Access::WRITE);
}

void ActiveClusterFilterPass::Render(RenderPipelineResourceManager& resource_manager) {
    PROFILE("ActiveClusterFilterPass");
    auto depth_buffer = resource_manager.GetResource<std::shared_ptr<RenderTexture2DResource>>(input_depth_buffer);
    auto list = Renderer::Get()->GetRenderCommandList();

    auto& world = Application::GetWorld();
    auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());

    ConfigBufferStruct config_buffer_struct = {};
    auto& window_props = Application::Get()->GetWindow()->GetProperties();
    config_buffer_struct.window_size = { window_props.resolution_x, window_props.resolution_y };
    config_buffer_struct.cluster_dimensions = { CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z};
    config_buffer_struct.depth_constant_a = camera.zFar / (camera.zFar - camera.zNear);
    config_buffer_struct.depth_constant_b = (-camera.zFar * camera.zNear) / (camera.zFar - camera.zNear);
    config_buffer_struct.near_plane = camera.zNear;
    config_buffer_struct.far_plane = camera.zFar;
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->config_buffer, &config_buffer_struct, sizeof(ConfigBufferStruct), 0);

    std::unique_ptr<uint32_t[]> zero(new uint32_t[CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z + 1]);
    memset(zero.get(), 0, sizeof(uint32_t) * (CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z + 1));
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->active_cluster_buffer, zero.get(), (CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z + 1) * sizeof(uint32_t), 0);

    list->SetPipeline(data->active_cluster_filter_pipeline);
    list->SetConstantBuffer("config_buffer", data->config_buffer);
    list->SetStorageBuffer("active_clusters", data->active_cluster_buffer);
    list->SetTexture2D("DepthBuffer", depth_buffer);
    list->Dispatch(CLUSTER_GRID_X, CLUSTER_GRID_Y, 1);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);

    resource_manager.SetResource<std::shared_ptr<RenderBufferResource>>(output_active_clusters, data->active_cluster_buffer);
}
