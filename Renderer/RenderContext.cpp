#include "RenderContext.h"

#include <platform/OpenGL/OpenGLRenderContext.h>


RenderContext* RenderContext::instance = nullptr;


RenderContext* RenderContext::Get()
{
	if (!instance) {
#ifdef OpenGL
		instance = new OpenGLRenderContext;
#else
		instance = nullptr;
#endif
	}
	return instance;
}
