#include "Renderer3D.h"
#include "MaterialManager.h"
#include <FrameManager.h>
#include "DeferredRenderingPipeline.h"
#include "Animations/AnimationManager.h"

Renderer3D* Renderer3D::instance = nullptr;

void Renderer3D::Init()
{
	if (!instance) {
		instance = new Renderer3D;
		MaterialManager::Init();
		AnimationManager::Init();
		instance->deferred_pipeline = DeferredRenderingPipeline::CreatePipeline();
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
	AnimationManager::Shutdown();
}

Renderer3D* Renderer3D::Get()
{
	return instance;
}

void Renderer3D::Update(float delta_time)
{
	MaterialManager::Get()->UpdateMaterials();
	deferred_pipeline->Render();
	default_descriptor_heap.FlushDescriptorDeallocations(FrameManager::Get()->GetCurrentFrameNumber());
}

Renderer3D::Renderer3D() : default_descriptor_heap(500), deferred_pipeline()
{

}
