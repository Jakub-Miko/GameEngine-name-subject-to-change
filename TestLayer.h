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

class TestLayer : public Layer
{
public:
    double counter = 0;
    bool stop = false;
    Entity entity1;
    Entity field[10][10];

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
       

    }

};