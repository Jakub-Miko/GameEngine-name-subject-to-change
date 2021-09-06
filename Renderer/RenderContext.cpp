#include "RenderContext.h"

#ifdef OpenGL
#include <R_API/OpenGL/OpenGLRenderContext.h>
#endif

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
