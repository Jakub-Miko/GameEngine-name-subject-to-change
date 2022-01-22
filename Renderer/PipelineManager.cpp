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
	for (auto layout : m_Layouts) {
		delete layout;
	}
}

PipelineManager::PipelineManager() : m_Layouts(), m_Layouts_mutex(), m_Signatures(), m_Signatures_mutex()
{
	m_Layouts.reserve(10);
	m_Signatures.reserve(10);
}

void PipelineManager::AddLayout(VertexLayout* layout)
{
	std::lock_guard<std::mutex> lock(m_Layouts_mutex);
	m_Layouts.push_back(layout);
}

void PipelineManager::AddSignature(RootSignature* signature)
{
	std::lock_guard<std::mutex> lock(m_Signatures_mutex);
	m_Signatures.push_back(signature);
}
