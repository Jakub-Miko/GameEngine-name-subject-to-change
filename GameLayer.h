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
#include <World/World.h>
#include <World/Systems/SquareRenderSystem.h>

class GameLayer : public Layer
{
public:
    World m_World;
public:

    World& GetWorld() { return m_World; }

    virtual void OnEvent(Event* e) override {

    }

    virtual void OnUpdate(float delta_time) override {
        SquareRenderSystem(m_World);
    }

};