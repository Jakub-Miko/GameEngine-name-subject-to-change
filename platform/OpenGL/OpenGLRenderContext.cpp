#include "OpenGLRenderContext.h"
#include <GL/glew.h>
#include "OpenGLRenderCommandQueue.h"
#include <Application.h>

void OpenGLRenderContext::Init()
{
    
}

void OpenGLRenderContext::PreInit()
{
    RenderCommandQueue* queue = new OpenGLRenderCommandQueue();
    SetRenderQueue(queue, RenderQueueTypes::DirectQueue);
    SetRenderQueue(queue, RenderQueueTypes::ComputeQueue);
    SetRenderQueue(queue, RenderQueueTypes::CopyQueue);
}

void OpenGLRenderContext::Destroy()
{
    delete Renderer::Get()->GetCommandQueue();
    SetRenderQueue(nullptr, RenderQueueTypes::DirectQueue);
    SetRenderQueue(nullptr, RenderQueueTypes::ComputeQueue);
    SetRenderQueue(nullptr, RenderQueueTypes::CopyQueue);
}

