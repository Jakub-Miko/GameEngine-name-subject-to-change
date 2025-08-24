#include "RenderFence.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderFence.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderFence.h>
#endif


RenderFence* RenderFence::CreateFence(uint32_t initial_value)
{
#ifdef OpenGL_API
	return new OpenGLRenderFence(initial_value);
#elif defined Vulkan_API
	return new VulkanRenderFence(initial_value);
#endif

}
