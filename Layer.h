#pragma once
class Layer {
public:
    virtual void OnUpdate(float delta_time) = 0;
    virtual ~Layer() {  };
};