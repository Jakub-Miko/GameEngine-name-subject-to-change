#include "PipelineManager.h"
#include <platform/OpenGL/OpenGLPipelineManager.h>

PipelineManager* PipelineManager::instance = nullptr;

void PipelineManager::Initialize()
{
	if (!instance) {
		instance = new OpenGLPipelineManager();
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
