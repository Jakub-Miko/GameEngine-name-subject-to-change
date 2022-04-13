
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
#include <World/Systems/BoxRenderer.h>
#include <Renderer/RenderDescriptorHeapBlock.h>
#include <Renderer/PipelineManager.h>
#include <Core/UnitConverter.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <Core/RuntimeTag.h>
#include <Window.h>
#include <FileManager.h>

class test1 {
    RuntimeTag("Test1")
};

class test2 {

};


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
    RenderDescriptorHeap* heap;
    std::shared_ptr<RenderTexture2DResource> color_texture;
    std::shared_ptr<RenderTexture2DResource> depth_texture;
    std::shared_ptr<RenderFrameBufferResource> frame_buffer;

    RenderDescriptorTable table1;
    RenderDescriptorTable table2;

    std::unique_ptr<TextureSampler> sampler;
    Pipeline* pipeline;
    Pipeline* pipeline_2;
    FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> resource2;
    glm::vec2 position = { 0,0 };
    glm::vec3 camerapos = {1.0f, 1.5f, -5.0f};

public:

    //Here we go a memory leak yay
    ~TestLayer() {
        //delete pipeline;
        //delete pipeline_2;
        Delete_Render_Box_data();
    }

    TestLayer() : Layer(), heap(new RenderDescriptorHeap(10)){
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
            else if (e->key_code == KeyCode::KEY_S && e->press_type == KeyPressType::KEY_PRESS) {
                Application::GetWorld().SaveScene("Entity_snapshot.json");
            }
            else if (e->key_code == KeyCode::KEY_L && e->press_type == KeyPressType::KEY_PRESS) {
                Application::GetWorld().LoadSceneFromFile("Entity_snapshot.json");
            }


            return false;
            });
        dispatch.Dispatch<MouseMoveEvent>([this](MouseMoveEvent* e) {
            glm::vec2 norm_scree_pos = UnitConverter::ScreenSpaceToNDC({ e->x,e->y });
            glm::vec2 offset = { -1.0,0.0 };
            norm_scree_pos += offset;
            norm_scree_pos.y *= -1;
            float distance = 5.0f;
            glm::vec3 pos{
                glm::cos(norm_scree_pos.x * glm::pi<float>()) * glm::cos(norm_scree_pos.y * glm::pi<float>()/2) ,
                glm::sin(norm_scree_pos.y * glm::pi<float>() / 2),
                glm::sin(norm_scree_pos.x * glm::pi<float>()) * glm::cos(norm_scree_pos.y * glm::pi<float>()/2)
            };
            camerapos = pos * distance;
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
            //TestDescriptor allocators
            //{
            //    RenderDescriptorHeapBlock* block = RenderDescriptorHeapBlock::CreateHeapBlock(20);
            //    RenderDescriptorAllocation* alloc_1 = block->Allocate(5);
            //    RenderDescriptorAllocation* alloc_2 = block->Allocate(10);
            //    RenderDescriptorAllocation* alloc_3 = block->Allocate(5);
            //    delete alloc_1;
            //    RenderDescriptorAllocation* alloc_4 = block->Allocate(10);
            //    delete alloc_2;
            //    block->FlushDescriptorDeallocations(FrameManager::Get()->GetCurrentFrameNumber());
            //    RenderDescriptorAllocation* alloc_5 = block->Allocate(10);
            //    delete alloc_3;
            //    delete alloc_5;
            //    block->FlushDescriptorDeallocations(FrameManager::Get()->GetCurrentFrameNumber());
            //    RenderDescriptorAllocation* alloc_6 = block->Allocate(20);
            //    delete alloc_6;
            //    block->FlushDescriptorDeallocations(FrameManager::Get()->GetCurrentFrameNumber());
            //    std::cout << "Stop";
            //}
            
            
           /* 
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

            table1 = heap->Allocate(2);

            RenderResourceManager::Get()->CreateConstantBufferDescriptor(table1, 0, resource);

            RenderResourceManager::Get()->CreateTexture2DDescriptor(table1, 1, texture);

            table2 = heap->Allocate(2);

            RenderResourceManager::Get()->CreateConstantBufferDescriptor(table2, 0, resource);

            RenderResourceManager::Get()->CreateTexture2DDescriptor(table2, 1, color_texture);

            Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
            stop = false;*/
            
            //Application::GetWorld().GetSceneGraph()->Deserialize(FileManager::Get()->GetAssetFilePath("save_file.json"));

            std::cout << RuntimeTag<TransformComponent>::GetName() << "\n";
            std::cout << RuntimeTag<CameraComponent>::GetName() << "\n";
            std::cout << RuntimeTag<int>::GetName() << "\n";

            DynamicPropertiesComponent comp1;
            comp1.m_Properties.insert(std::make_pair("string", "string"));
            comp1.m_Properties.insert(std::make_pair("int", 5.0));
            comp1.m_Properties.insert(std::make_pair("vec4", glm::vec4(1.0f)));

            nlohmann::json json;
            json["attrib"] = comp1;
            std::string dump = json.dump();
            std::cout << dump << "\n";
            DynamicPropertiesComponent comp2 = json["attrib"];

            stop = false;

        }
        
        
        auto props = Application::Get()->GetWindow()->m_Properties;
        auto camera = CameraComponent(45.0f, 0.1f, 1000.0f, (float)props.resolution_x / (float)props.resolution_y);
        auto view_matrix = glm::lookAt(camerapos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        Render_Box(BoundingBox({0.5f,0.5f,0.5f}), glm::translate(glm::mat4(1.0f), { 0.0f,0.6f,0.0f }), camera, view_matrix);
        Render_Box(BoundingBox(), glm::translate(glm::mat4(1.0f), { 0.0f,-0.6f,0.0f }), camera, view_matrix, PrimitivePolygonRenderMode::WIREFRAME);
        PROFILE("RenderRun");



        //auto list = Renderer::Get()->GetRenderCommandList();

        ////Implement Default RenderTarget attachments (renderbufferes in opengl case)
        //list->SetRenderTarget(frame_buffer);
        //list->SetPipeline(pipeline);
        //list->SetDescriptorTable("Test", table1);
        //list->SetIndexBuffer(resource_index);
        //list->SetVertexBuffer(resource_vertex);
        //list->Draw(6);


        //list->SetPipeline(pipeline_2);
        //list->SetDefaultRenderTarget();
        //list->SetDescriptorTable("Test", table2);
        //list->SetIndexBuffer(resource_index);
        //list->SetVertexBuffer(resource_vertex);
        //list->Draw(6);

        //Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);


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