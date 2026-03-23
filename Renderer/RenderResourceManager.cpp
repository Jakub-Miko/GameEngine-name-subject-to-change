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

void RenderResourceManager::RegisterGlobalResourceStore(const std::string& name,
	std::shared_ptr<RenderResourceStore> store) {
	global_resource_stores.insert(std::make_pair(name, store));
}

std::shared_ptr<RenderResourceStore> RenderResourceManager::GetGlobalResourceStore(const std::string& name) {
	auto fnd = global_resource_stores.find(name);
	if (fnd != global_resource_stores.end()) {
		return fnd->second;
	}
	else {
		return nullptr;
	}
}

void RenderResourceManager::UnregisterGlobalResourceStore(const std::string& name) {
	global_resource_stores.erase(name);
}
