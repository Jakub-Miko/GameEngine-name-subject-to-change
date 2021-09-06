#pragma once
#include "Layer.h"
#include "Application.h"
#include <iostream>


class GameLayer : public Layer
{
public:
    int counter = 0;
public:

    virtual void OnUpdate() override {
        auto queue = Application::Get()->GetRenderer()->GetRenderQueue();
        
        queue->DrawSquare(5, 2, 3, 4);
        queue->DrawSquare(8, 541, 12, 7);
        PROFILE("Submittion");
        queue->Submit();
    }

};