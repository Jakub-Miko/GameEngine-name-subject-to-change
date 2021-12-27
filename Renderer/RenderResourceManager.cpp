#include "RenderResourceManager.h"
#include <platform/OpenGL/OpenGLRenderResourceManager.h>

RenderResourceManager* RenderResourceManager::instance = nullptr;

void RenderResourceManager::Initialize()
{
	if (!instance) {
		instance = new OpenGLRenderResourceManager();
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
