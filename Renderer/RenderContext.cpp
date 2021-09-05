#include "RenderContext.h"
#include <R_API/OpenGL/OpenGLRenderContext.h>

RenderContext* RenderContext::instance = nullptr;


RenderContext* RenderContext::Get()
{
	if (!instance) {
		instance = new OpenGLRenderContext;
	}
	return instance;
}
