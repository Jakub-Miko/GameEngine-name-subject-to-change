#pragma once
#include "Layer.h"
#include "Application.h"
#include <iostream>
#include <TaskSystem.h>

class GameLayer : public Layer
{
public:
    int counter = 0;
public:

    virtual void OnUpdate() override {
        auto queue = Renderer::Get()->GetRenderCommandList();
        

        auto task = TaskSystem::Get()->CreateTask<bool>([&queue]() -> bool {
            PROFILE("Draw");
            queue->DrawSquare({ -0.5,0.5 }, { 0.75,0.5 }, { 1,0,0,1 });
            queue->DrawSquare({ 0.5,-0.5 }, { 0.5,0.75 });
            queue->DrawSquare({ 0.5,0.5 }, { 0.75,0.25 }, { 1,0,1,1 });
            queue->DrawSquare({ -0.5,-0.5 }, { 0.25,0.75 }, { 0,1,0,1 });

            return true;
            });

        TaskSystem::Get()->Submit(task);
        task->GetFuture().get();

        PROFILE("Submittion");
        queue->Submit();
    }

};