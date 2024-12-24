#include "RenderContext.h"
#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderContext.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderContext.h>
#endif

RenderContext* RenderContext::instance = nullptr;

RenderContext* RenderContext::Get()
{
	return instance;
}

void RenderContext::Create()
{
	if (!instance) {
#ifdef OpenGL_API
		instance = new OpenGLRenderContext();
#elif defined Vulkan_API
		instance = new VulkanRenderContext();
#endif
	}
}

void RenderContext::Shutdown()
{
	if (instance) {
		instance->Destroy();
		delete instance;
	}
}

void RenderContext::SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type)
{
	Renderer::Get()->SetRenderQueue(queue, type);
}
