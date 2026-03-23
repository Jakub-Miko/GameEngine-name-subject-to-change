#include "SkyboxPass.h"

#include "Application.h"
#include "dependencies/assimp/code/AssetLib/3DS/3DSHelper.h"
#include "dependencies/assimp/code/AssetLib/Collada/ColladaExporter.h"
#include "Renderer/MeshManager.h"
#include "Renderer/PipelineManager.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/Renderer3D/ClusteredRenderer/ClusteredLightCullingPass.h"
#include "VHACD/inc/vhacdMesh.h"
#include "World/Components/CameraComponent.h"
#include "World/Components/SkylightComponent.h"

struct SkyboxPass::internal_data {
    std::shared_ptr<Pipeline> skybox_pipeline;
    std::shared_ptr<RenderBufferResource> constant_scene_buf_bg;
    std::shared_ptr<RenderFrameBufferResource> frame_buffer;
    std::shared_ptr<Mesh> card_mesh;
};

struct LightingPassPreset;

template<>
struct VertexLayoutFactory<LightingPassPreset> {

    static VertexLayout* GetLayout() {
        static std::unique_ptr<VertexLayout> layout = nullptr;
        if (!layout) {
            VertexLayout* layout_new = new VertexLayout({
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "position"),
                });

            layout = std::unique_ptr<VertexLayout>(layout_new);
        }
        return layout.get();
    }

};

struct ConfigData {
    glm::mat4 projection_matrix;
    glm::mat4 inverse_view_matrix;
    glm::vec2 pixel_size;
    float depth_constant_a;
    float depth_constant_b;
    glm::uvec3 cluster_grid_size;
    int point_light_count;
    int directional_light_count;
    int skylight_count;
    float near_plane;
    float far_plane;
};

SkyboxPass::SkyboxPass(const std::string& input_color_buffer, const std::string& input_clustered_lights,
    const std::string& output_color_buffer) : input_color_buffer(input_color_buffer), input_clustered_lights(input_clustered_lights), output_color_buffer(output_color_buffer) {
    data = new internal_data;
    InitSkyboxPassData();
}

void SkyboxPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_color_buffer, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<ClusteredLightLists>(input_clustered_lights, RenderPassResourceDescriptor_Access::READ);
    setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_color_buffer, RenderPassResourceDescriptor_Access::WRITE);
}

void SkyboxPass::Render(RenderPipelineResourceManager& resource_manager) {
    auto& world = Application::GetWorld();
    auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
    auto& buffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_color_buffer);
    UpdateFramebuffer(buffer);
    camera.UpdateProjectionMatrix();
    auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
    auto projection = camera.GetProjectionMatrix();
    auto view_matrix = glm::inverse(camera_trans.TransformMatrix);
    auto skylight_view = Application::GetWorld().GetRegistry().view<SkylightComponent>();
    auto list = Renderer::Get()->GetRenderCommandList();

    SkylightComponent* bg_comp = nullptr;

    for (auto& ent : skylight_view) {
        Entity entity = Entity((uint32_t)ent);
        auto& light = world.GetComponent<SkylightComponent>(entity);

        if (!light.GetReflectionMap() || light.GetReflectionMap()->GetStatus() != ReflectionMapStatus::LOADED) {
            continue;
        }

        if (light.IsBackgroundVisible()) {
            bg_comp = &light;
        }
    }

    if (bg_comp) {
        view_matrix[3] = glm::vec4(0.0f);
        view_matrix[3][3] = 1.0f;
        auto color_in = bg_comp->GetLightColor();
        glm::mat4 inverse_view_projection = glm::inverse(projection * view_matrix);
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_bg, &inverse_view_projection, sizeof(glm::mat4), 0);
        RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_bg, glm::value_ptr(color_in), sizeof(glm::vec4), sizeof(glm::mat4));
        list->SetPipeline(data->skybox_pipeline);
        list->SetRenderTarget(data->frame_buffer);
        list->SetTexture2DCubemap("in_tex", bg_comp->GetReflectionMap()->GetSpecularMap());
        list->SetVertexBuffer(data->card_mesh->GetVertexBuffer());
        list->SetConstantBuffer("mvp", data->constant_scene_buf_bg);
        list->SetIndexBuffer(data->card_mesh->GetIndexBuffer());
        list->Draw(data->card_mesh->GetIndexCount());
    }

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
    resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_color_buffer, buffer);
}

SkyboxPass::~SkyboxPass() {
    if(data) {
        delete data;
    }
}

void SkyboxPass::InitSkyboxPassData() {
    GraphicsPipelineDescriptor pipeline_desc;
    pipeline_desc.viewport = RenderViewport();
    pipeline_desc.scissor_rect = RenderScissorRect();
    PipelineBlendFunctions blend_function;
    blend_function.dstAlpha = BlendFunction::ONE;
    blend_function.srcAlpha = BlendFunction::ONE;
    blend_function.srcRGB = BlendFunction::ONE;
    blend_function.dstRGB = BlendFunction::ONE;
    pipeline_desc.blend_functions = blend_function;
    pipeline_desc.enable_depth_clip = false;
    pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
    pipeline_desc.cull_mode = CullMode::FRONT;
    pipeline_desc.depth_function = DepthFunction::LESS_EQUAL;
    pipeline_desc.blend_equation = BlendEquation::ADD;
    pipeline_desc.layout = VertexLayoutFactory<LightingPassPreset>::GetLayout();
    pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/CubeMapRender.glsl");
    pipeline_desc.framebuffer_format.color_attachemt_formats = {
        { TextureFormat::RGBA_16FLOAT }
    };

    data->skybox_pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    RenderBufferDescriptor const_desc(sizeof(ConfigData), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
    data->constant_scene_buf_bg = RenderResourceManager::Get()->CreateBuffer(const_desc);

    struct Vertex {
        Vertex(glm::vec2 pos)
            : pos(pos) {}
        glm::vec2 pos;
    };

    Vertex card_vertecies[4] = {
        Vertex({-1,-1}),
        Vertex({-1,1}),
        Vertex({1,-1}),
        Vertex({1,1})
    };

    unsigned int indicies[6] = { 0,1,2,1,3,2 };

    RenderBufferDescriptor index_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
    RenderBufferDescriptor vertex_desc(sizeof(card_vertecies), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
    auto card_vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_desc);
    auto card_index_buffer = RenderResourceManager::Get()->CreateBuffer(index_desc);
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();

    RenderResourceManager::Get()->UploadDataToBuffer(list, card_vertex_buffer, (void*)card_vertecies, sizeof(card_vertecies),0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, card_index_buffer, (void*)indicies, sizeof(indicies),0);

    queue->ExecuteRenderCommandList(list);

    data->card_mesh = std::shared_ptr<Mesh>(new Mesh(card_vertex_buffer, card_index_buffer, 6));
}

void SkyboxPass::UpdateFramebuffer(std::shared_ptr<RenderFrameBufferResource> input_buffer) {
    if(data->frame_buffer) {
        auto& desc = data->frame_buffer->GetBufferDescriptor();
        if(desc.GetColorAttachmentAsTexture(0) == input_buffer->GetBufferDescriptor().GetColorAttachmentAsTexture(0) &&
            desc.GetDepthAttachmentAsTexture() == input_buffer->GetBufferDescriptor().GetDepthAttachmentAsTexture() ) {
            return;
        }
    }

    RenderFrameBufferDescriptor framebuffer_desc = {};
    framebuffer_desc.color_attachments = {{0, input_buffer->GetBufferDescriptor().GetColorAttachmentAsTexture(0)}};
    framebuffer_desc.depth_stencil_attachment = { 0,input_buffer->GetBufferDescriptor().GetDepthAttachmentAsTexture() };
    data->frame_buffer = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);
}
