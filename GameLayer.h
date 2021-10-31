#pragma once
#include "Layer.h"
#include "Application.h"
#include <iostream>
#include <TaskSystem.h>
#include <cmath>
#include <Promise.h>
#include <Renderer/Renderer.h>

class GameLayer : public Layer
{
public:
    double counter = 0;
public:

    virtual void OnUpdate(float delta_time) override {
        auto queue = Renderer::Get()->GetRenderCommandList();
        float speed = 0.005f;
        counter += speed * delta_time;
        float color = (std::sin(counter) + 1) /2;
        
        auto task = TaskSystem::Get()->CreateTask<bool>([&queue,color]() -> bool {
            PROFILE("Draw");
            queue->DrawSquare({ -0.5,0.5 }, { 0.75,0.5 }, { 1,0,0,1 });
            queue->DrawSquare({ 0.5,-0.5 }, { 0.5,0.75 });
            queue->DrawSquare({ 0.5,0.5 }, { 0.75,0.25 }, { 1,0,1,1 });
            queue->DrawSquare({ -0.5,-0.5 }, { 0.25,0.75 }, { 0,1,0,1 });
            queue->DrawSquare({ 0,0 }, { 0.25,0.25 }, { 1,color,0,1 });
            return true;
            });

        /*queue->DrawSquare({ -0.5,0.5 }, { 0.75,0.5 }, { 1,0,0,1 });
        queue->DrawSquare({ 0.5,-0.5 }, { 0.5,0.75 });
        queue->DrawSquare({ 0.5,0.5 }, { 0.75,0.25 }, { 1,0,1,1 });
        queue->DrawSquare({ -0.5,-0.5 }, { 0.25,0.75 }, { 0,1,0,1 });*/

        TaskSystem::Get()->Submit(task);
        task->GetFuture().GetValue();

        PROFILE("Submittion");
        Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(queue);
    }

};