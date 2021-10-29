#include "RenderContext.h"

#include <platform/OpenGL/OpenGLRenderContext.h>

RenderContext* RenderContext::instance = nullptr;

RenderContext* RenderContext::Get()
{
	return instance;
}

void RenderContext::Create()
{
	if (!instance) {
		instance = new OpenGLRenderContext();
	}
}

void RenderContext::Shutdown()
{
	if (instance) {
		instance->Destroy();
		delete instance;
	}
}

void RenderContext::SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type)
{
	Renderer::Get()->SetRenderQueue(queue, type);
}
