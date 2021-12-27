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
#include <Renderer/ShaderManager.h>
#include <Renderer/PipelineManager.h>

class TestLayer : public Layer
{
public:
    double counter = 0;
    bool stop = true;
    bool stop2 = true;
    Entity entity1;
    Entity field[10][10];
    std::shared_ptr<RenderBufferResource> resource;
    glm::vec2 position = { 0,0 };
public:



    TestLayer() : Layer() {
        glm::vec2 origin = { -4,-4 };
        for (int i = 0; i < 10; i++) {
            for (int y = 0; y < 10; y++) {
                int color = ((i * 9 + y) % 2 == 0) ? 1 : 0;
                Entity ent = Application::GetWorld().CreateEntity<SquareEntityType>(Entity(), glm::vec4( color,color,color,1 ), 
                    glm::vec2((origin + glm::vec2(i,y)) / glm::vec2(10.0,10.0)), 
                    glm::vec2( 0.1,0.1 ));
                //Application::GetWorld().SetComponent<SquareComponent>(ent, SquareComponent({ (origin + glm::vec2(i,y))/glm::vec2(10.0,10.0) }, { 0.1,0.1 }, {color,color,color,1}));

                field[i][y] = ent;
            }
        }
    }

    virtual void OnEvent(Event* e) override {

    }

    virtual void OnUpdate(float delta_time) override {
        if (stop == false && stop2 == true) {

            resource.reset();

            stop2 = false;
        }
        
        
        if (stop) {
            RenderBufferDescriptor desc(256, RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
            resource = RenderResourceManager::Get()->CreateBuffer(desc);


            unsigned char lol[32] = {
                1,5,8,4,2,6,4,56,
                1,5,8,4,2,6,4,56,
                1,5,8,4,2,6,4,56,
                1,5,8,4,2,6,4,56
            };
            auto list = Renderer::Get()->GetRenderCommandList();
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 0);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 32);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 64);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 96);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 128);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 128 + 32);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 128 + 64);
            RenderResourceManager::Get()->UploadDataToBuffer(list, resource, lol, sizeof(unsigned char) * 32, 128 + 96);


            PipelineDescriptor descr;

            Shader* shader = ShaderManager::Get()->GetShader("Default_shader.glsl");
            descr.shader = shader;

            RootSignature* sig = RootSignature<TestSig>::GetSignature();
            descr.root_signature = sig;
            
            descr.vertex_layout = VertexLayout<TestSig>::GetLayout();
            
            std::shared_ptr<Pipeline> pipeline = PipelineManager::Get()->CreatePipeline(descr);

            glm::vec4 value(1.0f);

            list->SetPipeline(pipeline);
            list->SetFloat4(pipeline->GetSlot("Constant_name"), &value);
            list->SetConstantBuffer(pipeline->GetSlot("Buffer_name"), resource);
            list->SetTable(pipeline->GetSlot("Table_name"), table);




            Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
            stop = false;
        }


        


    }

};