#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <Renderer/RenderCommandAllocator.h>
#include <Core/ExecutableCommand.h>

class Renderer;

class RenderCommandList : public ExecutableCommand
{
public:
    friend Renderer;

    RenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = {1.f,1.f,1.f,1.f}) = 0;


    virtual ~RenderCommandList() {};
protected:
    Renderer* m_Renderer;
    std::shared_ptr<RenderCommandAllocator> m_Alloc;
public:

    static RenderCommandList* CreateQueue(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);
};
