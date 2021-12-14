#pragma once
#include <Renderer/RenderCommandList.h>
#include <vector>

class OpenGLRenderCommand;
struct GLFWwindow;

class OpenGLRenderCommandList : public RenderCommandList {
public:
    friend class OpenGLRenderCommandQueue;
    OpenGLRenderCommandList(Renderer* renderer);
    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;


    virtual ~OpenGLRenderCommandList();
    
    void BindOpenGLContext();

private:

    template<typename T,typename ... Args>
    void PushCommand(Args&& ... args);

    virtual void Execute() override;

    OpenGLRenderCommand* m_Commands = nullptr;
    OpenGLRenderCommand* m_Commands_tail = nullptr;
    
};

