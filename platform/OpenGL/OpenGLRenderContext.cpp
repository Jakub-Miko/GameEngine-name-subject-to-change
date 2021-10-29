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
    SetRenderQueue(queue, DirectQueue);
    SetRenderQueue(queue, ComputeQueue);
    SetRenderQueue(queue, CopyQueue);
}

void OpenGLRenderContext::Destroy()
{
    delete Renderer::Get()->GetCommandQueue();
    SetRenderQueue(nullptr, DirectQueue);
    SetRenderQueue(nullptr, ComputeQueue);
    SetRenderQueue(nullptr, CopyQueue);
}

