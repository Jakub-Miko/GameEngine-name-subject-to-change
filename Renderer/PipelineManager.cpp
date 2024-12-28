#include "PipelineManager.h"
#include <Renderer/RootSignature.h>

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLPipelineManager.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanPipelineManager.h>
#endif

PipelineManager* PipelineManager::instance = nullptr;

void PipelineManager::Initialize()
{
	if (!instance) {
#ifdef OpenGL_API
		instance = new OpenGLPipelineManager();
#elif defined Vulkan_API
		instance = new VulkanPipelineManager();
#endif
	}
}

PipelineManager* PipelineManager::Get()
{
	return instance;
}

void PipelineManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

PipelineManager::~PipelineManager()
{

}

PipelineManager::PipelineManager() 
{

}

