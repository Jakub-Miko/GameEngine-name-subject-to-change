#pragma once
#include <memory>

class Renderer;

class RenderQueue
{
public:
    friend Renderer;

    RenderQueue(std::shared_ptr<Renderer> renderer);

    virtual void DrawSquare(float pos_x, float pos_y, float size_x, float size_y) = 0;

    virtual void Submit() = 0;


    virtual ~RenderQueue() {};
protected:
    std::shared_ptr<Renderer>  m_Renderer;
public:
    virtual void Execute() = 0;

    static RenderQueue* CreateQueue(std::shared_ptr<Renderer> renderer);
};
