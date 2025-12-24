#include "Renderer3D.h"
#include <Renderer/MaterialManager.h>
#include <FrameManager.h>

#include "ClusteredRendererPipeline.h"
#include "DeferredRenderingPipeline.h"
#include "Animations/AnimationManager.h"

Renderer3D* Renderer3D::instance = nullptr;

void Renderer3D::Init()
{
	if (!instance) {
		instance = new Renderer3D;
		MaterialManager::Init();
		AnimationManager::Init();
		instance->deferred_pipeline = ClusteredRendererPipeline::CreatePipeline();
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
}

Renderer3D::Renderer3D() : deferred_pipeline()
{

}
