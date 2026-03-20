#include "DebugOverlayPass.h"
#include "DebugOverlayPass.h"

#include "Application.h"
#include "ClusteredLightCullingPass.h"
#include "Window.h"
#include "Renderer/MaterialManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderResourceManager.h"
#include "World/Components/CameraComponent.h"

struct DebugOverlayPass::internal_data {
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<RenderTexture2DResource> output_overlay;
    std::shared_ptr<RenderFrameBufferResource> output_buffer;
    std::shared_ptr<RenderBufferResource> config_buffer;
    std::shared_ptr<RenderBufferResource> vertex_buffer;
    std::shared_ptr<RenderBufferResource> index_buffer;
};

struct ConfigBufferData {
    glm::mat4 projection_matrix;
    glm::vec2 pixel_size;
    float depth_constant_a;
    float depth_constant_b;
    glm::uvec3 cluster_grid_size;
    int light_count;
    float near_plane;
    float far_plane;
    float opacity;
    uint32_t mode;
};

enum class DebugOverlayMode {
    CLUSTER_GRID = 0,
    TILES = 1,
    LIGHT_COUNT = 2,
    RADIUS = 3,
    DEPTH_SLICE = 4
};

struct PostProcessPassPreset;

template<>
struct VertexLayoutFactory<PostProcessPassPreset> {

    static VertexLayout* GetLayout() {
        static std::unique_ptr<VertexLayout> layout = nullptr;
        if (!layout) {
            VertexLayout* layout_new = new VertexLayout({
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "position"),
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv")
                });

            layout = std::unique_ptr<VertexLayout>(layout_new);
        }
        return layout.get();
    }

};

DebugOverlayPass::DebugOverlayPass(const std::string& input_gbuffer, const std::string& input_gbuffer_material,
                                   const std::string& input_clustered_lights, const std::string& input_light_accum_texture,
                                   const std::string& output_overlay) :
    input_gbuffer(input_gbuffer), input_gbuffer_material(input_gbuffer_material),
    input_clustered_lights(input_clustered_lights), input_light_accum_texture(input_light_accum_texture),
    output_overlay(output_overlay) {
    InitPass();
}

void DebugOverlayPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<ClusteredLightLists>(input_clustered_lights, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<std::shared_ptr<RenderTexture2DResource>>(input_light_accum_texture, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<std::shared_ptr<Material>>(input_gbuffer_material, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<std::shared_ptr<RenderTexture2DResource>>(output_overlay, RenderPassResourceDescriptor_Access::WRITE);

    overlay_opacity_prop = setup_builder.GetProperties()->SetProperty("Overlay Opacity", 0.5f).second;
    enable_debug_overlay_prop = setup_builder.GetProperties()->SetProperty("Enable post process overlay", true).second;
    debug_layer_mode_prop = setup_builder.GetProperties()->SetProperty("Debug overlay mode", MultiChoice( {
        "Cluster grid", "Tiles", "Light count", "Radius", "Depth slice"
    }, "Cluster grid")).second;
}

void DebugOverlayPass::Render(RenderPipelineResourceManager& resource_manager) {
    if(enable_debug_overlay_prop->GetValueTyped() == false) return;

    auto& world = Application::GetWorld();
    auto gbuffer_material = resource_manager.GetResource<std::shared_ptr<Material>>(input_gbuffer_material);
    auto list = Renderer::Get()->GetRenderCommandList();
    auto& input_light_accum = resource_manager.GetResource<std::shared_ptr<RenderTexture2DResource>>(input_light_accum_texture);
    auto& clustered_lights = resource_manager.GetResource<ClusteredLightLists>(input_clustered_lights);

    auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());

    ConfigBufferData config_buffer_data = {};
    config_buffer_data.opacity = overlay_opacity_prop->GetValueTyped();
    config_buffer_data.cluster_grid_size = glm::uvec3(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
    config_buffer_data.light_count = clustered_lights.num_of_point_lights;
    config_buffer_data.depth_constant_a = camera.zFar / (camera.zFar - camera.zNear);;
    config_buffer_data.depth_constant_b = (-camera.zFar * camera.zNear) / (camera.zFar - camera.zNear);
    config_buffer_data.far_plane = camera.zFar;
    config_buffer_data.near_plane = camera.zNear;
    config_buffer_data.projection_matrix = camera.GetProjectionMatrix();
    config_buffer_data.mode = debug_layer_mode_prop->GetValueTyped().GetIndex();
    config_buffer_data.pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
        1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };

    RenderResourceManager::Get()->UploadDataToBuffer(list, data->config_buffer, &config_buffer_data, sizeof(ConfigBufferData), 0);

    list->SetPipeline(data->pipeline);
    list->SetRenderTarget(data->output_buffer);
    list->SetMaterial("GBufferMaterial", gbuffer_material);
    list->SetStorageBuffer("light_buffer", clustered_lights.point_light_buffer);
    list->SetStorageBuffer("light_assignment_buffer", clustered_lights.light_assignment_buffer);
    list->SetStorageBuffer("cluster_buffer", clustered_lights.cluster_buffer);
    list->SetTexture2D("light_accum_buffer", input_light_accum);
    list->SetConstantBuffer("conf", data->config_buffer);
    list->SetVertexBuffer(data->vertex_buffer);
    list->SetIndexBuffer(data->index_buffer);

    list->Draw(6);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
    resource_manager.SetResource<std::shared_ptr<RenderTexture2DResource>>(output_overlay,data->output_overlay);
}

DebugOverlayPass::~DebugOverlayPass() {
    delete data;
}

void DebugOverlayPass::InitPass() {
    data = new internal_data;

    GraphicsPipelineDescriptor pipeline_desc = {};
    pipeline_desc.layout = VertexLayoutFactory<PostProcessPassPreset>::GetLayout();
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/DebugOverlay.glsl");
    pipeline_desc.framebuffer_format.color_attachemt_formats = {
        { TextureFormat::BGRA_SRGB }
    };

    data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    TextureSamplerDescritor sampler_desc;
    sampler_desc.AddressMode_U = TextureAddressMode::BORDER;
    sampler_desc.AddressMode_V = TextureAddressMode::BORDER;
    sampler_desc.AddressMode_W = TextureAddressMode::BORDER;
    sampler_desc.border_color = glm::vec4(1.0, 0.4, 1.0, 1.0);
    sampler_desc.filter = TextureFilter::POINT_MIN_MAG;
    sampler_desc.LOD_bias = 0;
    sampler_desc.min_LOD = 0;
    sampler_desc.max_LOD = 0;

    RenderTexture2DDescriptor output_overlay_desc = {};
    output_overlay_desc.format = TextureFormat::BGRA_SRGB;
    output_overlay_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
    output_overlay_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
    output_overlay_desc.usage = TextureUsage::COLOR_ATTACHMENT_READABLE;
    output_overlay_desc.sampler = TextureSampler::CreateSampler(sampler_desc);

    data->output_overlay = RenderResourceManager::Get()->CreateTexture(output_overlay_desc);

    RenderFrameBufferDescriptor framebuffer_desc;
    framebuffer_desc.color_attachments = { {0,data->output_overlay} };
    data->output_buffer = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);

    RenderBufferDescriptor config_buffer_desc(sizeof(ConfigBufferData), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);
    data->config_buffer = RenderResourceManager::Get()->CreateBuffer(config_buffer_desc);

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    Vertex vertecies[4] = {
        Vertex{{-1,-1},{0,0}},
        Vertex{{-1,1},{0,1}},
        Vertex{{1,-1},{1,0}},
        Vertex{{1,1},{1,1}}
    };
    unsigned int indicies[6] = { 0,1,2,1,3,2 };

    RenderBufferDescriptor index_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
    RenderBufferDescriptor vertex_desc(sizeof(vertecies), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
    data->vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_desc);
    data->index_buffer = RenderResourceManager::Get()->CreateBuffer(index_desc);
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();

    RenderResourceManager::Get()->UploadDataToBuffer(list, data->vertex_buffer,vertecies, sizeof(vertecies),0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, data->index_buffer,indicies, sizeof(indicies),0);

    queue->ExecuteRenderCommandList(list);
}
