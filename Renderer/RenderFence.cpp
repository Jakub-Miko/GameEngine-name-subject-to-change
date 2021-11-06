#include "RenderFence.h"
#include <platform/OpenGL/OpenGLRenderFence.h>

RenderFence* RenderFence::CreateFence()
{
	return new OpenGLRenderFence();
}
