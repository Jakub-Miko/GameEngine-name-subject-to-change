#include "RenderResourceManager.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderResourceManager.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderResourceManager.h>
#endif

RenderResourceManager* RenderResourceManager::instance = nullptr;

void RenderResourceManager::Initialize()
{
	if (!instance) {
#ifdef OpenGL_API
		instance = new OpenGLRenderResourceManager();
#elif defined Vulkan_API
		instance = new VulkanRenderResourceManager();
#endif
	}
}

RenderResourceManager* RenderResourceManager::Get()
{
	return instance;
}

void RenderResourceManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}
