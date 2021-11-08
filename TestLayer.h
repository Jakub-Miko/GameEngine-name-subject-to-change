#pragma once
#include "Layer.h"
#include "Application.h"
#include <iostream>
#include <TaskSystem.h>
#include <cmath>
#include <Promise.h>
#include <Renderer/Renderer.h>
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
    Entity entities[5];
    glm::vec2 position = { 0,0 };
public:



    TestLayer() : Layer() {
        SquareComponent init_vals[5] = {
               SquareComponent({ -0.5,0.5 }, { 0.75,0.5 }, { 1,0,0,1 }),
               SquareComponent({ 0.5,-0.5 }, { 0.5,0.75 }),
               SquareComponent({ 0.5,0.5 }, { 0.75,0.25 }, { 1,0,1,1 }),
               SquareComponent({ -0.5,-0.5 }, { 0.25,0.75 }, { 0,1,0,1 }),
               SquareComponent({0,0}, { 0.25,0.25 }, { 1,0,0,1 })
        };

        for (int i = 0; i < 5; i++) {
            entities[i] = Application::Get()->GetWorld().CreateEntity();
            Application::Get()->GetWorld().SetComponent<SquareComponent>(entities[i], init_vals[i]);
        }
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
                    position = glm::vec2(1, -1) * position2;
                    return true;
                }
                return false;
            });

        dispatcher.Dispatch<MouseMoveEvent>([this](MouseMoveEvent* e) {
            glm::vec2 position2 = { e->x, e->y };
            position2.x /= 800 / 2;
            position2.y /= 600 / 2;
            position2.x -= 1;
            position2.y -= 1;
            position = glm::vec2(1, -1) * position2;
            return true;
            });
    }

    virtual void OnUpdate(float delta_time) override {
        float speed = 0.005f;
        if (!stop) {
            counter += speed * delta_time;
        }
        float color = (std::sin(counter) + 1) / 2;

        Application::Get()->GetWorld().GetComponent<SquareComponent>(entities[4]) = SquareComponent(position, { 0.25,0.25 }, { 1,color,0,1 });


    }

};