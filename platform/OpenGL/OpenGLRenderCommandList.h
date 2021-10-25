#pragma once
#include <Renderer/RenderCommandList.h>
#include <vector>

class OpenGLRenderCommand;

class OpenGLRenderCommandList : public RenderCommandList {
public:
    OpenGLRenderCommandList(Renderer* renderer, RenderCommandAllocator* alloc);
    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void Submit() override;
    virtual ~OpenGLRenderCommandList();
private:
    virtual void Execute() override;
    std::vector<OpenGLRenderCommand*> m_Commands;
    
};