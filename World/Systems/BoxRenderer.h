#pragma once
#include <Renderer/Renderer.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/CameraComponent.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/ShaderManager.h>
#include <glm/glm.hpp>
#include <Core/FrameMultiBufferResource.h>

struct Render_Box_data {
    
    struct Constant_buffer_type {
        glm::mat4 mvp_matrix;
        glm::vec4 sun_direction;
        glm::vec4 color;
    };

    Render_Box_data() : pipeline(),
    vertex_buffer(),
    index_buffer(),
    constant_buffer() {}

    void clear() {
        vertex_buffer.reset();
        index_buffer.reset();
        constant_buffer = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>();
    }

    Render_Box_data(const CameraComponent& camera, const glm::mat4& camera_transform) {
        float vertices[] = {
        0.5f, -0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f,  0.0f, -1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,   0.0f,  0.0f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   1.0f,  0.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f, 1.0f,   0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f,  0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f, -0.5f, 1.0f,   0.0f,  1.0f,  0.0f, 1.0f
        };

        unsigned int indicies[] = {
            0 ,
            1 ,
            2 ,
            3 ,
            4 ,
            5 ,
            6 ,
            7 ,
            8 ,
            9 ,
            10,
            11,
            12,
            13,
            14,
            15,
            16,
            17,
            18,
            19,
            20,
            21,
            22,
            23,
            24,
            25,
            26,
            27,
            28,
            29,
            30,
            31,
            32,
            33,
            34,
            35
        };



        auto command_list = Renderer::Get()->GetRenderCommandList();
        auto command_queue = Renderer::Get()->GetCommandQueue();

        PipelineDescriptor pipeline_desc;
        pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
        pipeline_desc.layout = VertexLayoutFactory<BoxPreset>::GetLayout();
        pipeline_desc.scissor_rect = RenderScissorRect();
        pipeline_desc.viewport = RenderViewport();
        pipeline_desc.signature = RootSignatureFactory<BoxPreset>::GetRootSignature();
        pipeline_desc.shader = ShaderManager::Get()->GetShader("BoxShader.glsl");

        pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

        PipelineDescriptor pipeline_desc_wireframe;
        pipeline_desc_wireframe.flags = PipelineFlags::ENABLE_DEPTH_TEST;
        pipeline_desc_wireframe.layout = VertexLayoutFactory<BoxPreset>::GetLayout();
        pipeline_desc_wireframe.scissor_rect = RenderScissorRect();
        pipeline_desc_wireframe.viewport = RenderViewport();
        pipeline_desc_wireframe.signature = RootSignatureFactory<BoxPreset>::GetRootSignature();
        pipeline_desc_wireframe.shader = ShaderManager::Get()->GetShader("BoxShader.glsl");
        pipeline_desc_wireframe.polygon_render_mode = PrimitivePolygonRenderMode::WIREFRAME;

        pipeline_wireframe = PipelineManager::Get()->CreatePipeline(pipeline_desc_wireframe);

        RenderBufferDescriptor vertex_buffer_desc(sizeof(vertices), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);

        vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, vertex_buffer, vertices, sizeof(vertices), 0);

        RenderBufferDescriptor index_buffer_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);

        index_buffer = RenderResourceManager::Get()->CreateBuffer(index_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, index_buffer, indicies, sizeof(indicies), 0);


        Constant_buffer_type constant_buf_data = {
            camera.GetProjectionMatrix() * camera_transform,
            glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
            glm::vec4(0.3f,0.7f,1.0f,1.0f)
        };

        

        constant_buffer = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([&]() -> std::shared_ptr<RenderBufferResource>  {
            RenderBufferDescriptor constant_buffer_desc(sizeof(constant_buf_data), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);

            std::shared_ptr<RenderBufferResource> constant_buffer_instance = RenderResourceManager::Get()->CreateBuffer(constant_buffer_desc);
            RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer_instance, &constant_buf_data, sizeof(constant_buf_data), 0);
            return constant_buffer_instance;
            });

        command_queue->ExecuteRenderCommandList(command_list);
    };

    ~Render_Box_data() {
        
    }
    
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Pipeline> pipeline_wireframe;
    std::shared_ptr<RenderBufferResource> vertex_buffer;
    std::shared_ptr<RenderBufferResource> index_buffer;
    FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> constant_buffer;
};

Render_Box_data& Get_Render_Box_data() {
    static Render_Box_data data(CameraComponent(45.0f, 0.1f, 1000.0f, 1.0f), glm::lookAt(glm::vec3(1.0f, 1.5f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    return data;
}

void Delete_Render_Box_data() {
    Get_Render_Box_data().clear();
}

void Render_Box(const glm::mat4& model_matrix,const CameraComponent& camera, const glm::mat4& camera_transform, PrimitivePolygonRenderMode render_mode = PrimitivePolygonRenderMode::DEFAULT) {
    auto command_list = Renderer::Get()->GetRenderCommandList();
    auto command_queue = Renderer::Get()->GetCommandQueue();
	
    Render_Box_data& data = Get_Render_Box_data();

    Render_Box_data::Constant_buffer_type buffer = {
         camera.GetProjectionMatrix() * camera_transform * model_matrix,
            glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
            glm::vec4(0.3f,0.7f,1.0f,1.0f)
    };

    RenderResourceManager::Get()->UploadDataToBuffer(command_list, data.constant_buffer.GetResource(), &buffer, sizeof(buffer), 0);

    command_list->SetDefaultRenderTarget();
    switch (render_mode) {
    case PrimitivePolygonRenderMode::DEFAULT:
        command_list->SetPipeline(data.pipeline);
        break;
    case PrimitivePolygonRenderMode::WIREFRAME:
        command_list->SetPipeline(data.pipeline_wireframe);
        break;
    }
    command_list->SetVertexBuffer(data.vertex_buffer);
    command_list->SetIndexBuffer(data.index_buffer);
    command_list->SetConstantBuffer("input", data.constant_buffer.GetResource());
    command_list->Draw(36);

    command_queue->ExecuteRenderCommandList(command_list);

}