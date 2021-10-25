#include "RenderCommandQueue.h"
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>

RenderCommandQueue* RenderCommandQueue::CreateRenderCommandQueue()
{
	return new OpenGLRenderCommandQueue();
}
