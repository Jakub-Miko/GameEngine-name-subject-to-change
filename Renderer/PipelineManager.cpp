#include "PipelineManager.h"
#include <platform/OpenGL/OpenGLPipelineManager.h>
#include <Renderer/RootSignature.h>

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

PipelineManager::~PipelineManager()
{
	for (auto sig : m_Signatures) {
		delete sig;
	}
}

PipelineManager::PipelineManager() : m_Layouts() , m_Signatures()
{
	m_Layouts.reserve(10);
	m_Signatures.reserve(10);
}
