#include "RenderContext.h"

#include <platform/OpenGL/OpenGLRenderContext.h>



RenderContext* RenderContext::Get()
{
#ifdef OpenGL
	static OpenGLRenderContext instance;
	return &instance;
#else
	return nullptr;
#endif
}
