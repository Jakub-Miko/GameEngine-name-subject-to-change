#pragma once
#include <memory>
#include <glm/glm.hpp>

class Renderer;

class RenderQueue
{
public:
    friend Renderer;

    RenderQueue(std::shared_ptr<Renderer> renderer);

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = {1.f,1.f,1.f,1.f}) = 0;

    virtual void Submit() = 0;


    virtual ~RenderQueue() {};
protected:
    std::shared_ptr<Renderer>  m_Renderer;
public:
    virtual void Execute() = 0;

    static RenderQueue* CreateQueue(std::shared_ptr<Renderer> renderer);
};
