       
#pragma once
#include "Layer.h"
#include <Renderer/TextRenderer.h>
#include <Core/Debug.h>
#include <Renderer/Renderer3D/Animations/AnimationManager.h>
#include "Application.h"
#include <Core/ModuleManager.h>
#include <iostream>
#include <World/Components/SerializableComponent.h>
#include <World/Components/UITextComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/SkylightComponent.h>
#include <World/Components/AudioComponent.h>
#include <Input/Input.h>
#include <Renderer/TextureManager.h>
#include <Audio/AudioSystem.h>
#include <AL/al.h>
#include <TestModule/TestFunc.h>
#include <Events/SubjectObserver.h>
#include <glm/glm.hpp>
#include <Events/Event.h>
#include <Core/RuntimeTag.h>
#include <Window.h>
#include <FileManager.h>
#ifdef EDITOR
#include <Editor/Editor.h>
#include <imgui.h>
#endif

class TestEventType : public Event {
    EVENT_ID(TestEventType);

public:
    int x, y;
};

class TestLayer : public Layer
{
public:
    double counter = 0;
    bool stop = true;
    bool stop2 = true;
    bool playing = false;
    Entity skeletal_ent;
    Entity AudioEntity;
    //Entity entity1;
    //Entity mesh_enity;
    //Entity second_ent;
    //Entity ent_sphere;
    //Entity field[10][10];
    //std::shared_ptr<RenderBufferResource> resource;
    //std::shared_ptr<RenderBufferResource> resource_vertex;
    //std::shared_ptr<RenderBufferResource> resource_index;
    //Future<std::shared_ptr<RenderTexture2DResource>> texture;
    //
    //std::shared_ptr<RenderTexture2DResource> color_texture;
    //std::shared_ptr<RenderTexture2DResource> depth_texture;
    //std::shared_ptr<RenderFrameBufferResource> frame_buffer;
    //BoundingBox box;
    //RenderDescriptorTable table1;
    //RenderDescriptorTable table2;
    //size_t index_count;
    //Entity entity_box;
    //Future<std::shared_ptr<Mesh>> mesh;
    //OrientedBoundingBox oriented_box;

    //FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> constant_buffer;

    EventObserverBase* test_observer;

    //std::shared_ptr<TextureSampler> sampler;
    //std::shared_ptr<Pipeline> pipeline;
    //FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> resource2;
    //glm::vec2 position = { 0,0 };
    //glm::vec3 camerapos = {1.0f, 1.5f, -5.0f};

public:

    //Here we go a memory leak yay
    ~TestLayer() {
        //delete pipeline;
        //delete pipeline_2; 
        //Delete_Render_Box_data();
        delete test_observer;
       
    }

    TestLayer() : Layer(){
        glm::vec2 origin = { -4,-4 };
            //std::cout << "Test Event: " << e->x << ", " << e->y << "\n";
        test_observer = MakeEventObserver<TestEventType>([this](TestEventType* e) -> bool { 
            std::cout << "Test Event: " << e->x << ", " << e->y << "\n";
            return false; });
        Application::Get()->RegisterObserver<TestEventType>((EventObserverBase*)test_observer);

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
        //EventDispacher dispatch(e);
        //dispatch.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
        //    if (e->key_code == KeyCode::KEY_R && e->press_type == KeyPressType::KEY_PRESS) {
        //        Application::GetWorld().GetPhysicsEngine().PassiveMode();
        //    }
        //    else if (e->key_code == KeyCode::KEY_T && e->press_type == KeyPressType::KEY_PRESS) {
        //        Application::GetWorld().GetPhysicsEngine().ActiveMode();
        //    }
        //    else if (e->key_code == KeyCode::KEY_S && e->press_type == KeyPressType::KEY_PRESS) {
        //        Application::GetWorld().SaveScene("Entity_snapshot.json");
        //    }
        //    else if (e->key_code == KeyCode::KEY_L && e->press_type == KeyPressType::KEY_PRESS) {
        //        Application::GetWorld().LoadSceneFromFile("Entity_snapshot.json");
        //    }
        //    else if (e->key_code == KeyCode::KEY_Q && e->press_type == KeyPressType::KEY_PRESS) {
        //        
        //        
        //        Application::Get()->Exit();
        //    }
        //    else if (e->key_code == KeyCode::KEY_KP_5 && e->press_type == KeyPressType::KEY_PRESS) {
        //        TextureManager::Get()->ReleaseTexture("asset:Heaven.png"_path);
        //        texture.GetValue().reset();
        //        texture.~Future();
        //    }

        //    else if (e->key_code == KeyCode::KEY_KP_2 && e->press_type == KeyPressType::KEY_PRESS) {
        //        texture = TextureManager::Get()->LoadTextureFromFileAsync("asset:Heaven.png"_path, false);
        //    }


        //    return false;
        //    });
        //dispatch.Dispatch<MouseMoveEvent>([this](MouseMoveEvent* e) {
        //    glm::vec2 norm_scree_pos = UnitConverter::ScreenSpaceToNDC({ e->x,e->y });
        //    glm::vec2 offset = { -1.0,0.0 };
        //    norm_scree_pos += offset;
        //    norm_scree_pos.y *= -1;
        //    float distance = 5.0f;
        //    glm::vec3 pos{
        //        glm::cos(norm_scree_pos.x * glm::pi<float>()) * glm::cos(norm_scree_pos.y * glm::pi<float>()/2) ,
        //        glm::sin(norm_scree_pos.y * glm::pi<float>() / 2),
        //        glm::sin(norm_scree_pos.x * glm::pi<float>()) * glm::cos(norm_scree_pos.y * glm::pi<float>()/2)
        //    };
        //    camerapos = pos * distance;
        //    //camerapos = glm::vec3(0.0, 0.0, -5.0);
        //    return false;
        //    });
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

          //  std::cout << RuntimeTag<TransformComponent>::GetName() << "\n";
          //  std::cout << RuntimeTag<CameraComponent>::GetName() << "\n";
          //  std::cout << RuntimeTag<int>::GetName() << "\n";

          //  DynamicPropertiesComponent comp1;
          //  comp1.m_Properties.insert(std::make_pair("string", "string"));
          //  comp1.m_Properties.insert(std::make_pair("int", 5.0));
          //  comp1.m_Properties.insert(std::make_pair("vec4", glm::vec4(1.0f)));

          //  nlohmann::json json;
          //  json["attrib"] = comp1;
          //  std::string dump = json.dump();
          //  std::cout << dump << "\n";
          //  DynamicPropertiesComponent comp2 = json["attrib"];



          //  

          //  auto command_list = Renderer::Get()->GetRenderCommandList();
          //  auto command_queue = Renderer::Get()->GetCommandQueue();

          //  PipelineDescriptor pipeline_desc;
          //  pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
          //  pipeline_desc.layout = VertexLayoutFactory<MeshPreset>::GetLayout();
          //  pipeline_desc.scissor_rect = RenderScissorRect();
          //  pipeline_desc.viewport = RenderViewport();
          //  pipeline_desc.shader = ShaderManager::Get()->GetShader("MeshShader.glsl");

          //  pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

          //  Render_Box_data::Constant_buffer_type constant_buf_data = {
          //      glm::mat4(1.0f),
          //      glm::mat4(1.0f),
          //      glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
          //      glm::vec4(0.3f,0.7f,1.0f,1.0f),
          //      glm::vec4(0,0,0,0)
          //  };



          //  constant_buffer = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([&]() -> std::shared_ptr<RenderBufferResource> {
          //      RenderBufferDescriptor constant_buffer_desc(sizeof(constant_buf_data), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);

          //      std::shared_ptr<RenderBufferResource> constant_buffer_instance = RenderResourceManager::Get()->CreateBuffer(constant_buffer_desc);
          //      RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer_instance, &constant_buf_data, sizeof(constant_buf_data), 0);
          //      return constant_buffer_instance;
          //      });

          //  command_queue->ExecuteRenderCommandList(command_list);
          //  
          //  mesh_enity = Application::GetWorld().CreateEntity();
          //  Application::GetWorld().SetComponent<MeshComponent>(mesh_enity,"asset:Pillar_with_uvs.mesh"_path);
          //  Application::GetWorld().SetEntityTranslation(mesh_enity, glm::vec3(0.0f, -5.0f, 0.0f));
          //  
          //  TextureSamplerDescritor desc_sample;
          //  desc_sample.AddressMode_U = TextureAddressMode::BORDER;
          //  desc_sample.AddressMode_V = TextureAddressMode::BORDER;
          //  desc_sample.AddressMode_W = TextureAddressMode::BORDER;
          //  desc_sample.border_color = glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);
          //  desc_sample.filter = TextureFilter::POINT_MIN_MAG_MIP;
          //  desc_sample.LOD_bias = 0.0f;
          //  desc_sample.max_LOD = 10;
          //  desc_sample.min_LOD = 0;

          //  TextureManager::Get()->MakeTextureFromImageAsync("asset:Heaven.png"_path, "asset:image_texture.tex"_path, desc_sample);

          //  texture = TextureManager::Get()->LoadTextureFromFileAsync("asset:image_texture.tex", false);
          //  
          //  auto& index = Application::GetWorld().GetSpatialIndex();
          //  Entity entity_1 = Application::GetWorld().CreateEntity();
          ///*  Application::GetWorld().SetComponent<BoundingVolumeComponent>(entity_1, BoundingBox());*/
          //  Application::GetWorld().SetEntityTranslation(entity_1, { -1.9,-1.7,-1.9 });
          //  Entity entity_2 = Application::GetWorld().CreateEntity();
          //  /*Application::GetWorld().SetComponent<BoundingVolumeComponent>(entity_2, BoundingBox()); */
          //  Application::GetWorld().SetEntityTranslation(entity_2, { -1.9,-1.7,-1.9 });
          //  Application::GetWorld().GetSceneGraph()->CalculateMatricies();

          //  auto ent_1 = Application::GetWorld().CreateEntity();
          //  Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent_1, BoundingBox({ 1,10,1 }, { 0,0,0 }));
          //  Application::GetWorld().GetSceneGraph()->CalculateMatricies();
            //index.AddEntity(ent_1);
            //index.AddEntity(entity_1);
            //index.AddEntity(entity_2);
            //index->RemoveEntity(entity_2);

#pragma region RenderPassBuilder
                
   /*         std::cout << "Id: " << RuntimeTag<RenderResourceCollection<bool>>::GetId() << " Name: " << RuntimeTag<RenderResourceCollection<bool>>::GetName() << "\n";
            std::cout << "Id: " << RuntimeTag<RenderResourceCollection<glm::vec2>>::GetId() << " Name: " << RuntimeTag<RenderResourceCollection<glm::vec2>>::GetName() << "\n";
            std::cout << "Name:" << RuntimeTag<glm::vec2*>::GetName() << " Id: " << RuntimeTag<glm::vec2*>::GetId() << "\n";
            std::cout << "Name: " << RuntimeTag<bool*>::GetName() << " Id: " << RuntimeTag<bool*>::GetId() << "\n";


*/

#pragma endregion
            std::cout << "Size of uvec4: " << sizeof(glm::uvec4) << "\n";
            TestEventType ev = TestEventType();
            ev.x = 5;
            ev.y = 8;
            Application::Get()->SendObservedEvent(&ev);
           /* skeletal_ent = Application::GetWorld().CreateEntity();
            Application::GetWorld().SetEntitySkeletalMesh(skeletal_ent,"asset:dancing_vampire.skel"_path, "asset:dancing_vampire_animations/default.anim"_path);
            Application::GetWorld().SetComponent< SerializableComponent>(skeletal_ent);
          */
            
#pragma region AudioTest
            
           /* audio_source = AudioSystem::Get()->CreateAudioSource();
            audio_source->SetAudioObject(AudioSystem::Get()->GetAudioObject("asset:test_track_mono.wav"_path));
            audio_source->SetLooping(false);
            audio_source->SetSourcePosition({ -100.0f,10.0f,10.0f });
            audio_source->Play();*/



#pragma endregion

            auto cubemap = TextureManager::Get()->GetReflectionMap("asset:HDR_texture.tex");


#pragma region ModuleTest

            decltype(&TestMe) test_function = ModuleManager::Get()->LoadModule("TestLib")->GetSymbol<decltype(TestMe)>("TestMe");
            std::cout << "Running Module Start\n";
            test_function();
            std::cout << "Running Module Stop\n";


#pragma endregion



            stop = false;

        }
        //RenderPassBuilder builder;
        //builder.AddPass(new TestPass2);
        //builder.AddPass(new TestPass1);
        //builder.AddPass(new TestPass4);
        //builder.AddPass(new TestPass3);
        //auto pipeline = builder.Build();
        //pipeline.Render(); 

        //auto& transform_box = Application::GetWorld().GetComponent<TransformComponent>(entity_box);
        //oriented_box.center = transform_box.translation;
        //oriented_box.rotation_matrix = glm::toMat3(transform_box.rotation);
        //oriented_box.size = transform_box.size*2.0f;
        //Application::GetWorld().SetEntityRotation(entity_box, glm::toMat3(transform_box.rotation));


        //std::vector<Entity> entities_vector;
        //Application::GetWorld().GetSpatialIndex().BoxCulling(Application::GetWorld(), oriented_box, entities_vector);

        //for (auto ent : entities_vector) {
        //    std::cout << ent.id << "\n";
        //}

      /*  auto& index = Application::GetWorld().GetSpatialIndex();
        std::vector<Entity> entities;
        Application::GetWorld().GetComponent<CameraComponent>(Application::GetWorld().GetPrimaryEntity()).UpdateViewFrustum(Application::GetWorld().GetComponent<TransformComponent>(Application::GetWorld().GetPrimaryEntity()).TransformMatrix);
        index.FrustumCulling(Application::GetWorld(), Application::GetWorld().GetComponent<CameraComponent>(Application::GetWorld().GetPrimaryEntity()).GetViewFrustum(), entities);
        index.Visualize();*/
        //for (auto ent : entities) {
        //    std::cout << ent.id << ", ";
        //}
       /* if (Application::GetWorld().EntityExists(mesh_enity) && Application::GetWorld().HasComponent<MeshComponent>(mesh_enity)  && Application::GetWorld().GetPrimaryEntity() != Entity()) {
            auto& camera = Application::GetWorld().GetComponent<CameraComponent>(Application::GetWorld().GetPrimaryEntity());
            auto& trans = Application::GetWorld().GetComponent<TransformComponent>(Application::GetWorld().GetPrimaryEntity());
            auto& mesh_mesh = Application::GetWorld().GetComponent<MeshComponent>(mesh_enity);
            auto& mesh_transform = Application::GetWorld().GetComponent<TransformComponent>(mesh_enity);

            auto mesh_m = mesh_mesh.mesh;

            auto command_list = Renderer::Get()->GetRenderCommandList();
            auto command_queue = Renderer::Get()->GetCommandQueue();

            Render_Box_data& data = Get_Render_Box_data();

            Render_Box_data::Constant_buffer_type buffer = {
                 (camera.GetProjectionMatrix() * glm::inverse(trans.TransformMatrix)) * mesh_transform.TransformMatrix,
                 mesh_transform.TransformMatrix,
                    glm::normalize(glm::vec4(0.20f, 1.0f, -3.0f,0.0f)),
                    glm::vec4(0.6f,0.6f,0.6f,1.0f),
                    glm::vec4(0,0,0,0)
            };
             
            RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer.GetResource(), &buffer, sizeof(buffer), 0);

            command_list->SetDefaultRenderTarget();
            command_list->SetPipeline(pipeline);
            command_list->SetVertexBuffer(mesh_m->GetVertexBuffer());
            command_list->SetIndexBuffer(mesh_m->GetIndexBuffer());
            command_list->SetConstantBuffer("conf", constant_buffer.GetResource());
            command_list->Draw(mesh_m->GetIndexCount());
            
            command_queue->ExecuteRenderCommandList(command_list);

            std::string path1 = "asset:lol1.txt"_path;
            std::string path2 = "engine_asset:lol5.txt"_path;
            std::string path3 = "api:lol18.txt"_path;
            std::string path4 = "lol1.txt"_path;

        }*/


        
        //std::cout << std::endl;

       /* glm::vec3 pos = camerapos;
        glm::quat rot = glm::quatLookAt(-glm::normalize(pos ), glm::vec3(0, 1, 0));

        glm::mat4 rotate =  glm::toMat4(rot);
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 result =  glm::inverse(translate * rotate);


        glm::mat4 third_box = glm::transpose(glm::transpose(glm::translate(glm::mat4(1.0f), pos)) * glm::toMat4(rot));
        third_box = glm::inverse(third_box);
        

        auto view_matrix = glm::lookAt(camerapos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        Render_Box(BoundingBox({0.5f,0.5f,0.5f}), glm::translate(glm::mat4(1.0f), { 0.0f,0.6f,0.0f }), camera, result);
        Render_Box(BoundingBox(), glm::translate(glm::mat4(1.0f), { 0.0f,-0.6f,0.0f }), camera, result, PrimitivePolygonRenderMode::WIREFRAME);
        Render_Box(BoundingBox(), third_box, camera, result, PrimitivePolygonRenderMode::WIREFRAME);*/
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