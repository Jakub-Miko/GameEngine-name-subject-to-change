#pragma once
#include <Renderer/RenderQueue.h>
#include <vector>

class OpenGLRenderCommand;

class OpenGLRenderQueue : public RenderQueue {
public:
    OpenGLRenderQueue(std::shared_ptr<Renderer> renderer);
    virtual void DrawSquare(float pos_x, float pos_y, float size_x, float size_y) override;
    virtual void Submit() override;
    virtual ~OpenGLRenderQueue();
private:
    virtual void Execute() override;
    std::vector<OpenGLRenderCommand*> m_Commands;
    
};