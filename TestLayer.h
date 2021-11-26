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
                Entity ent = Application::GetWorld().CreateEntity();
                Application::GetWorld().SetComponent<SquareComponent>(ent, SquareComponent({ (origin + glm::vec2(i,y))/glm::vec2(10.0,10.0) }, { 0.1,0.1 }, {color,color,color,1}));

                field[i][y] = ent;
            }
        }
        entity1 = Application::GetWorld().CreateEntity<SquareEntityType>(glm::vec4(1, 1, 0, 1),glm::vec2(0,0),glm::vec2(0.25,0.25));
        Application::GetWorld().SetComponent<ScriptComponent>(entity1, ScriptComponent("SecondScript.lua"));
    }

    virtual void OnEvent(Event* e) override {
        EventDispacher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e)
            {
                if (e->key_code == KeyCode::KEY_SPACE && e->press_type == KeyPressType::KEY_PRESS) {
                    this->stop = !this->stop;
                    return true;
                }
                return false;
            });
        dispatcher.Dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent* e)
            {
                if (e->press_type == KeyPressType::KEY_PRESS && e->key_code == MouseButtonCode::MOUSE_BUTTON_LEFT) {
                    glm::vec2 position2 = Input::Get()->GetMoutePosition();
                    position2.x /= 800 / 2;
                    position2.y /= 600 / 2;
                    position2.x -= 1;
                    position2.y -= 1;
                    auto entity = Application::GetWorld().CreateEntity();
                    Application::GetWorld().SetComponent<SquareComponent>(entity, SquareComponent({ glm::vec2(1, -1) * position2 }, { 0.1,0.1 }, { 0,1,1,1 }));
                    Application::GetWorld().SetComponent<ScriptComponent>(entity, ScriptComponent("TestScript.lua"));
                    return true;
                }
                return false;
            });
    }

    virtual void OnUpdate(float delta_time) override {
        float speed = 0.005f;
        if (!stop) {
            counter += speed * delta_time;
        }
        float color = (std::sin(counter) + 1) / 2;

        Application::GetWorld().GetComponent<SquareComponent>(entity1) = SquareComponent(position, { 0.25,0.25 }, { 1,color,0,1 });


    }

};