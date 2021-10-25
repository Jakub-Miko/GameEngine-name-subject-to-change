#include "RenderCommandAllocator.h"
#include <platform/OpenGL/OpenGLRenderCommandAllocator.h>

RenderCommandAllocator* RenderCommandAllocator::CreateAllocator(size_t starting_size)
{
	return new OpenGLRenderCommandAllocator(starting_size);
}
