#include "RenderFence.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderFence.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderFence.h>
#endif


RenderFence* RenderFence::CreateFence()
{
#ifdef OpenGL_API
	return new OpenGLRenderFence();
#elif defined Vulkan_API
	return new VulkanRenderFence();
#endif

}
