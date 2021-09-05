#pragma once
class Layer {
public:
    virtual void OnUpdate() = 0;
    virtual ~Layer() {  };
};