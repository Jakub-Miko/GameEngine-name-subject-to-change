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
        
        queue->DrawSquare({ -0.5,0.5 }, { 0.75,0.5 }, {1,0,0,1});
        queue->DrawSquare({ 0.5,-0.5 }, { 0.5,0.75 });
        queue->DrawSquare({ 0.5,0.5 }, { 0.75,0.25 }, { 1,0,1,1 });
        PROFILE("Submittion");
        queue->Submit();
    }

};