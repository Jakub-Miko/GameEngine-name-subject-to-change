#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <Renderer/RenderCommandAllocator.h>

class Renderer;

class RenderCommandList
{
public:
    friend Renderer;

    RenderCommandList(Renderer* renderer, RenderCommandAllocator* alloc);

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = {1.f,1.f,1.f,1.f}) = 0;

    virtual void Submit() = 0;

    virtual ~RenderCommandList() {};
protected:
    Renderer* m_Renderer;
    RenderCommandAllocator* m_Alloc;
public:
    virtual void Execute() = 0;

    static RenderCommandList* CreateQueue(Renderer* renderer, RenderCommandAllocator* alloc);
};
