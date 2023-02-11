#include "MeshRenderSystem.h"
#include <World/System.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/ScriptComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Systems/BoxRenderer.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/TextureManager.h>
#include <Renderer/Renderer3D/MaterialManager.h>

struct const_buf_type {
    glm::mat4 mvp_matrix;
    glm::mat4 model;
};

struct mesh_data {
    std::shared_ptr<Pipeline> pipeline;
    FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> constant_buffer;
};

static mesh_data* local_data = nullptr;

static void RenderMesh(MeshComponent& component, Entity ent) {
    auto& camera = Application::GetWorld().GetComponent<CameraComponent>(Application::GetWorld().GetPrimaryEntity());
    auto& trans = Application::GetWorld().GetComponent<TransformComponent>(Application::GetWorld().GetPrimaryEntity());
    auto& mesh_mesh = component;
    auto& mesh_transform = Application::GetWorld().GetComponent<TransformComponent>(ent);

    auto mesh_m = mesh_mesh.GetMesh();

    auto command_list = Renderer::Get()->GetRenderCommandList();
    auto command_queue = Renderer::Get()->GetCommandQueue();

    Render_Box_data& data = *Get_Render_Box_data();

    const_buf_type buffer = {
         (camera.GetProjectionMatrix() * glm::inverse(trans.TransformMatrix)) * mesh_transform.TransformMatrix,
         mesh_transform.TransformMatrix
    };

    RenderResourceManager::Get()->UploadDataToBuffer(command_list, local_data->constant_buffer.GetResource(), &buffer, sizeof(buffer), 0);

    auto material = MaterialManager::Get()->GetMaterial("asset:Material1.mat");

    command_list->SetDefaultRenderTarget();
    command_list->SetPipeline(local_data->pipeline);

    material->SetMaterial(command_list, local_data->pipeline);

    command_list->SetVertexBuffer(mesh_m->GetVertexBuffer());
    command_list->SetIndexBuffer(mesh_m->GetIndexBuffer());
    command_list->SetConstantBuffer("conf", local_data->constant_buffer.GetResource());
    command_list->Draw(mesh_m->GetIndexCount());

    command_queue->ExecuteRenderCommandList(command_list);
}

void MeshRenderSystem(World& world) {
    auto func_1 = [&world](ComponentCollection compcol, system_view_type<MeshComponent>& comps, entt::registry* reg) {
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            if (reg->all_of<MeshComponent>(*iter)) {
                auto comp = reg->get<MeshComponent>(*iter);
                RenderMesh(comp, *iter);
            }
        };

    };

    RunSystemSimple<MeshComponent>(world, func_1);
}

void InitMeshRenderSystem()
{
    if (local_data) {
        return;
    }
    Render_Box_data::Constant_buffer_type constant_buf_data = {
                glm::mat4(1.0f),
                glm::mat4(1.0f),
                glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
                glm::vec4(0.3f,0.7f,1.0f,1.0f),
                glm::vec4(0,0,0,0)
    };
    auto command_list = Renderer::Get()->GetRenderCommandList();
    auto command_queue = Renderer::Get()->GetCommandQueue();

    FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> constant_buf;
    constant_buf = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([&]() -> std::shared_ptr<RenderBufferResource> {
        RenderBufferDescriptor constant_buffer_desc(sizeof(constant_buf_data), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);

        std::shared_ptr<RenderBufferResource> constant_buffer_instance = RenderResourceManager::Get()->CreateBuffer(constant_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer_instance, &constant_buf_data, sizeof(constant_buf_data), 0);
        return constant_buffer_instance;
        });

    std::shared_ptr<Pipeline> pipeline;

    PipelineDescriptor pipeline_desc;
    pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
    pipeline_desc.layout = VertexLayoutFactory<MeshPreset>::GetLayout();
    pipeline_desc.scissor_rect = RenderScissorRect();
    pipeline_desc.viewport = RenderViewport();
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/MeshShader.glsl");

    pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    local_data = new mesh_data{ pipeline, std::move(constant_buf) };
    command_queue->ExecuteRenderCommandList(command_list);


}

void SutdownMeshRenderSystem()
{
    if (local_data) {
        delete local_data;
    }
}
