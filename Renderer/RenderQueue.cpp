#include "RenderQueue.h"
#include "platform/OpenGL/OpenGLRenderQueue.h"
#include <Renderer/Renderer.h>


RenderQueue::RenderQueue(std::shared_ptr<Renderer> renderer)
    :m_Renderer(renderer)
{
}

RenderQueue* RenderQueue::CreateQueue(std::shared_ptr<Renderer> renderer) {
    return new OpenGLRenderQueue(renderer);
}
