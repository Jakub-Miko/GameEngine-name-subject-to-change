#include "RenderCommandAllocator.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderCommandAllocator.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderCommandAllocator.h>
#endif


RenderCommandAllocator* RenderCommandAllocator::CreateAllocator(size_t starting_size)
{
#ifdef OpenGL_API
	return new OpenGLRenderCommandAllocator(starting_size);
#elif defined Vulkan_API
	return new VulkanRenderCommandAllocator(starting_size);
#endif
}
