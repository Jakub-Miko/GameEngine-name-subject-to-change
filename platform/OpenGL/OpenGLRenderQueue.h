#pragma once
#include <Renderer/RenderQueue.h>
#include <vector>

class OpenGLRenderCommand;

class OpenGLRenderQueue : public RenderQueue {
public:
    OpenGLRenderQueue(std::shared_ptr<Renderer> renderer);
    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void Submit() override;
    virtual ~OpenGLRenderQueue();
private:
    virtual void Execute() override;
    std::vector<OpenGLRenderCommand*> m_Commands;
    
};