#pragma once
#include <Renderer/RenderCommandList.h>
#include <vector>

class OpenGLRenderCommand;
struct GLFWwindow;

class OpenGLRenderCommandList : public RenderCommandList {
public:
    friend class OpenGLRenderCommandQueue;
    OpenGLRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);
    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual ~OpenGLRenderCommandList();


    void BindOpenGLContext();
private:
    virtual void Execute() override;
    OpenGLRenderCommand* m_Commands = nullptr;
    
};