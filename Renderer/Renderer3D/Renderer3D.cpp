#include "Renderer3D.h"
#include <Renderer/MaterialManager.h>
#include <FrameManager.h>

#include "DeferredClusteredRendererPipeline.h"
#include "DeferredRenderingPipeline.h"
#include "ForwardClusteredRendererPipeline.h"
#include "SkeletalAnimations/SkeletalAnimationManager.h"

Renderer3D* Renderer3D::instance = nullptr;

void Renderer3D::Init()
{
	if (!instance) {
		instance = new Renderer3D;
		MaterialManager::Init();
		SkeletalAnimationManager::Init();
		instance->RegisterPipeline("DeferredClustered", DeferredClusteredRendererPipeline::CreatePipeline());
		instance->RegisterPipeline("ForwardClustered", ForwardClusteredRendererPipeline::CreatePipeline());
		instance->SetActivePipeline("DeferredClustered");
	}
}

void Renderer3D::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

void Renderer3D::PreShutdown()
{
	MaterialManager::Shutdown();
	SkeletalAnimationManager::Shutdown();
}

Renderer3D* Renderer3D::Get()
{
	return instance;
}

void Renderer3D::Update(float delta_time)
{
	MaterialManager::Get()->UpdateMaterials();
	current_pipeline.second->Render();
}

void Renderer3D::RegisterPipeline(const std::string& name, std::shared_ptr<RenderPipeline> pipeline) {
	rendering_pipelines.insert(std::make_pair(name, pipeline));
}

void Renderer3D::SetActivePipeline(const std::string& name) {
	auto fnd = rendering_pipelines.find(name);
	if (fnd != rendering_pipelines.end()) {
		current_pipeline = std::make_pair(fnd->first, fnd->second);
	}
}

void Renderer3D::RemovePipeline(const std::string& name) {
	auto fnd = rendering_pipelines.find(name);
	if (fnd != rendering_pipelines.end()) {
		if(fnd->second == current_pipeline.second) {
			throw std::runtime_error("Cannot remove active pipeline");
		}
		rendering_pipelines.erase(fnd);
	}
}

Renderer3D::Renderer3D()
{

}
