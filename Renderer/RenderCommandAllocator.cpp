#include "RenderCommandAllocator.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderCommandAllocator.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderCommandAllocator.h>
#endif


std::shared_ptr<RenderCommandAllocator> RenderCommandAllocator::CreateAllocator(size_t starting_size)
{
#ifdef OpenGL_API
	return std::make_shared<OpenGLRenderCommandAllocator>(starting_size);
#elif defined Vulkan_API
	return std::make_shared<VulkanRenderCommandAllocator>(starting_size);
#endif
}
