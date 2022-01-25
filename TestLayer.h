#pragma once
#include "Layer.h"
#include "Application.h"
#include <iostream>
#include <TaskSystem.h>
#include <cmath>
#include <Promise.h>
#include <Renderer/Renderer.h>
#include <World/Components/ScriptComponent.h>
#include <World/Components/SquareComponent.h>
#include <Input/Input.h>
#include <Events/KeyPressEvent.h>
#include <Events/MouseButtonPressEvent.h>
#include <Events/MouseMoveEvent.h>
#include <glm/glm.hpp>
#include <Renderer/RenderResourceManager.h>
#include <stb_image.h>
#include <Renderer/ShaderManager.h>
#include <Core/FrameMultiBufferResource.h>
#include <Renderer/RootSignature.h>
#include <Renderer/PipelineManager.h>
#include <FileManager.h>

class TestLayer : public Layer
{
public:
    double counter = 0;
    bool stop = true;
    bool stop2 = true;
    Entity entity1;
    Entity field[10][10];
    std::shared_ptr<RenderBufferResource> resource;
    std::shared_ptr<RenderBufferResource> resource_vertex;
    std::shared_ptr<RenderBufferResource> resource_index;
    std::shared_ptr<RenderTexture2DResource> texture;

    std::shared_ptr<RenderTexture2DResource> color_texture;
    std::shared_ptr<RenderTexture2DResource> depth_texture;
    std::shared_ptr<RenderFrameBufferResource> frame_buffer;

    std::unique_ptr<TextureSampler> sampler;
    Pipeline* pipeline;
    Pipeline* pipeline_2;
    FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> resource2;
    glm::vec2 position = { 0,0 };
public:

    ~TestLayer() {
        delete pipeline;
        delete pipeline_2;
    }

    TestLayer() : Layer() {
        glm::vec2 origin = { -4,-4 };
        //for (int i = 0; i < 10; i++) {
        //    for (int y = 0; y < 10; y++) {
        //        int color = ((i * 9 + y) % 2 == 0) ? 1 : 0;
        //        Entity ent = Application::GetWorld().CreateEntity<SquareEntityType>(Entity(), glm::vec4( color,color,color,1 ), 
        //            glm::vec2((origin + glm::vec2(i,y)) / glm::vec2(10.0,10.0)), 
        //            glm::vec2( 0.1,0.1 ));
        //        Application::GetWorld().SetComponent<SquareComponent>(ent, SquareComponent({ (origin + glm::vec2(i,y))/glm::vec2(10.0,10.0) }, { 0.1,0.1 }, {color,color,color,1}));

        //        field[i][y] = ent;
        //    }
        //}
    }

    virtual void OnEvent(Event* e) override {
        EventDispacher dispatch(e);
        dispatch.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
            if (e->key_code == KeyCode::KEY_R && e->press_type == KeyPressType::KEY_PRESS) {
                resource2 = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>();
            }
            return false;
            });
    }

    virtual void OnUpdate(float delta_time) override {
        //if (stop == false && stop2 == true) {

        //    resource.reset();

        //    stop2 = false;
        //}
        
        #pragma region Test1


        if (stop) {
            PROFILE("RendererInit");
            struct data_type {
                glm::vec4 color;
                glm::vec2 scale;
            };
            
            
            struct Vertex {
                glm::vec2 pos;
                glm::vec2 uv;
            };

            auto list = Renderer::Get()->GetRenderCommandList();

            TextureSamplerDescritor desc_sample;
            desc_sample.AddressMode_U = TextureAddressMode::BORDER;
            desc_sample.AddressMode_V = TextureAddressMode::BORDER;
            desc_sample.AddressMode_W = TextureAddressMode::BORDER;
            desc_sample.border_color = glm::vec4(0.2f,0.7f,1.0f,1.0f);
            desc_sample.filter = TextureFilter::POINT_MIN_MAG_MIP;
            desc_sample.LOD_bias = 0.0f;
            desc_sample.max_LOD = 10;
            desc_sample.min_LOD = 0;

            sampler.reset(TextureSampler::CreateSampler(desc_sample));

            texture = RenderResourceManager::Get()->CreateTextureFromFile(list, "test_image.jpg", sampler.get());

            RenderResourceManager::Get()->GenerateMIPs(list, texture);
            data_type d;
            d.color = glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);
            d.scale = glm::vec2((float)(texture->GetBufferDescriptor().width)/(float)(texture->GetBufferDescriptor().height), 1.0f);


            Vertex pos[4] = {
                {{-0.5f,-0.5f}, {0.0f,0.0f}},
                {{0.5f,-0.5f}, {1.0f,0.0f}},
                {{0.5f,0.5f}, {1.0f,1.0f}},
                {{-0.5f,0.5f}, {0.0f,1.0f}},
            };

            unsigned int ind[6] = {
                0,1,2,
                0,2,3
            };

            RenderBufferDescriptor desc_ver(sizeof(pos), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
            resource_vertex = RenderResourceManager::Get()->CreateBuffer(desc_ver);

            RenderBufferDescriptor desc_ind(sizeof(ind), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
            resource_index = RenderResourceManager::Get()->CreateBuffer(desc_ind);
 
            RenderBufferDescriptor desc(sizeof(d), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);
            resource = RenderResourceManager::Get()->CreateBuffer(desc);

            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, &d, sizeof(d), 0);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource_vertex, &pos, sizeof(pos), 0);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource_index, &ind, sizeof(ind), 0);

            PipelineDescriptor descr;
            descr.shader = ShaderManager::Get()->GetShader("Default_shader.glsl");
            descr.signature = RootSignatureFactory<TestPreset>::GetRootSignature();
            descr.layout = VertexLayoutFactory<TestPreset>::GetLayout();
            pipeline = PipelineManager::Get()->CreatePipeline(descr);

            PipelineDescriptor descr_2;
            descr_2.shader = ShaderManager::Get()->GetShader("Default_shader_2.glsl");
            descr_2.signature = RootSignatureFactory<TestPreset>::GetRootSignature();
            descr_2.layout = VertexLayoutFactory<TestPreset>::GetLayout();
            pipeline_2 = PipelineManager::Get()->CreatePipeline(descr_2);

            RenderTexture2DDescriptor color_desc;
            color_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
            color_desc.height = 600;
            color_desc.width = 800;
            color_desc.sampler = sampler.get();

            RenderTexture2DDescriptor depth_desc;
            depth_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
            depth_desc.height = 600;
            depth_desc.width = 800;
            depth_desc.sampler = sampler.get();

            color_texture = RenderResourceManager::Get()->CreateTexture(color_desc);
            depth_texture = RenderResourceManager::Get()->CreateTexture(depth_desc);

            RenderFrameBufferDescriptor framebuffer_desc;
            framebuffer_desc.color_attachments = { color_texture };
            framebuffer_desc.depth_stencil_attachment = depth_texture;

            frame_buffer = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);



            Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
            stop = false;

        }
        PROFILE("RenderRun");

        auto list = Renderer::Get()->GetRenderCommandList();

        //Implement Default RenderTarget attachments (renderbufferes in opengl case)
        list->SetRenderTarget(frame_buffer);
        list->SetPipeline(pipeline);
        list->SetConstantBuffer("Testblock", resource);
        list->SetTexture2D("TestTexture", texture);
        list->SetIndexBuffer(resource_index);
        list->SetVertexBuffer(resource_vertex);
        list->Draw(6);


        list->SetPipeline(pipeline_2);
        list->SetDefaultRenderTarget();
        list->SetConstantBuffer("Testblock", resource);
        list->SetTexture2D("TestTexture", color_texture);
        list->SetIndexBuffer(resource_index);
        list->SetVertexBuffer(resource_vertex);
        list->Draw(6);

        Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);


#pragma endregion

        #pragma region Test2

        //if (stop) {

        //    RenderBufferDescriptor desc(128, RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);

        //    resource2 = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([desc]() {

        //        return RenderResourceManager::Get()->CreateBuffer(desc);

        //        });
        //    stop = false;
        //}
        #pragma endregion



    }

};