#pragma once
#include "Layer.h"
#include <World/Components/ScriptComponent.h>

class GameLayer : public Layer
{
public:


    virtual void OnEvent(Event* e) override;


    //Required for load system
    void LoadSystem();
    
    //Required for initialization.
    void PreUpdate(float delta_time);

    virtual void OnUpdate(float delta_time) override;

};