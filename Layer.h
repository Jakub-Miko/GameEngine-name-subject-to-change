#pragma once
class Event;
class Layer {
public:
    virtual void OnEvent(Event* e) {};
    virtual void OnUpdate(float delta_time) = 0;

    virtual ~Layer() {  };
};