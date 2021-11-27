#pragma once
#include "Layer.h"
#include "Application.h"
#include <iostream>
#include <TaskSystem.h>
#include <cmath>
#include <Promise.h>
#include <Renderer/Renderer.h>
#include <Input/Input.h>
#include <World/Systems/ScriptSystem.h>
#include <Events/KeyPressEvent.h>
#include <Events/MouseButtonPressEvent.h>
#include <Events/MouseMoveEvent.h>
#include <glm/glm.hpp>
#include <World/World.h>
#include <World/Systems/SquareRenderSystem.h>
#include <World/Systems/InitializationSystem.h>
#include <World/Systems/KeyPressedScriptSystem.h>
#include <World/Systems/MousePressedScriptSystem.h>

class GameLayer : public Layer
{
public:
    World m_World;
public:

    World& GetWorld() { return m_World; }

    virtual void OnEvent(Event* e) override {
        EventDispacher dispatch(e);
        dispatch.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
            KeyPressedScriptSystem(m_World, e);
            return false;
            });
        
        dispatch.Dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent* e) {
            MousePressedScriptSystem(m_World, e);
            return false;
            });


    }

    virtual void OnUpdate(float delta_time) override {
        InitializationSystem(m_World);
        ScriptSystem(m_World,delta_time);
        SquareRenderSystem(m_World);
    }

};