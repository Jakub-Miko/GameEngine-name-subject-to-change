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

    RenderCommandList(Renderer* renderer);
    virtual ~RenderCommandList() {};

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = {1.f,1.f,1.f,1.f}) = 0;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) = 0;

protected:

    Renderer* m_Renderer;

public:
    static RenderCommandList* CreateQueue(Renderer* renderer);
};
