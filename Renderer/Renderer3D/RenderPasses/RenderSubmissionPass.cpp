#include "RenderSubmissionPass.h"
#include <World/World.h>
#include <World/SpatialIndex.h>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/RenderResourceCollection.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/MeshComponent.h>
#include <Application.h>

RenderSubmissionPass::RenderSubmissionPass(const std::string& output_collection_name) : output_collection_name(output_collection_name)
{

}

void RenderSubmissionPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<std::shared_ptr<Mesh>>>(output_collection_name, RenderPassResourceDescriptor_Access::WRITE);
}

void RenderSubmissionPass::Render(RenderPipelineResourceManager& resource_manager)
{
	RenderResourceCollection<std::shared_ptr<Mesh>> collection;
	World& world = Application::GetWorld();
	Entity camera_entity = world.GetPrimaryEntity();
	if (camera_entity == Entity()) {
		resource_manager.SetResource(output_collection_name, collection);
		return;
	}

	CameraComponent& camera = world.GetComponent<CameraComponent>(camera_entity);
	camera.UpdateProjectionMatrix();
	camera.UpdateViewFrustum(world.GetComponent<TransformComponent>(camera_entity).TransformMatrix);
	const Frustum& camera_frustum = camera.GetViewFrustum();
	std::vector<Entity> visible;
	world.GetSpatialIndex().FrustumCulling(world, camera_frustum, visible);
	for (auto ent : visible) {
		if (world.HasComponent<MeshComponent>(ent)) {
			collection.resources.push_back(world.GetComponent<MeshComponent>(ent).mesh);
		}
	}
	resource_manager.SetResource(output_collection_name, std::move(collection));
}
