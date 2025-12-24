#include "RenderSubmissionPass.h"
#include <World/World.h>
#include <World/SpatialIndex.h>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/RenderResourceCollection.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <Application.h>

RenderSubmissionPass::RenderSubmissionPass(const std::string& output_mesh_name, const std::string& output_skeletal_mesh_name, const std::string& output_light_name,
	const std::string& output_shadowed_directional_light_name, const std::string& output_shadowed_point_light_name)
	: output_mesh_name(output_mesh_name), output_light_name(output_light_name), output_shadowed_directional_light_name(output_shadowed_directional_light_name), output_shadowed_point_light_name(output_shadowed_point_light_name),
	output_skeletal_mesh_name(output_skeletal_mesh_name)
{

}

void RenderSubmissionPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(output_mesh_name, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(output_skeletal_mesh_name, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(output_light_name, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(output_shadowed_directional_light_name, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(output_shadowed_point_light_name, RenderPassResourceDescriptor_Access::WRITE);
}

void RenderSubmissionPass::Render(RenderPipelineResourceManager& resource_manager)
{
	RenderResourceCollection<Entity> collection_meshes;
	RenderResourceCollection<Entity> collection_skeletal_meshes;
	RenderResourceCollection<Entity> collection_lights;
	RenderResourceCollection<Entity> collection_directional_shadowed_lights;
	RenderResourceCollection<Entity> collection_point_shadowed_lights;
	World& world = Application::GetWorld();
	Entity camera_entity = world.GetPrimaryEntity();
	if (camera_entity == Entity()) {
		resource_manager.SetResource(output_mesh_name, collection_meshes);
		resource_manager.SetResource(output_mesh_name, collection_meshes);
		resource_manager.SetResource(output_skeletal_mesh_name, collection_skeletal_meshes);
		return;
	}

	CameraComponent& camera = world.GetComponent<CameraComponent>(camera_entity);
	camera.UpdateProjectionMatrix();
	camera.UpdateViewFrustum(world.GetComponent<TransformComponent>(camera_entity).TransformMatrix);
	const Frustum& camera_frustum = camera.GetViewFrustum();
	std::vector<Entity> visible;
	world.GetSpatialIndex().FrustumCulling(world, camera_frustum, visible);
	for (auto ent : visible) {
		if (world.HasComponent<MeshComponent>(ent) && world.GetComponent<MeshComponent>(ent).GetVisibility()) {
			collection_meshes.resources.push_back(ent);
		}
		else if (world.HasComponent<SkeletalMeshComponent>(ent) && world.GetComponent<SkeletalMeshComponent>(ent).GetVisibility()) {
			collection_skeletal_meshes.resources.push_back(ent);
		}
		if (world.HasComponent<LightComponent>(ent)) {
			if (world.HasComponent<ShadowCasterComponent>(ent)) {
				switch (world.GetComponent<LightComponent>(ent).type) {
				case LightType::DIRECTIONAL:
					collection_directional_shadowed_lights.resources.push_back(ent);
					break;
				case LightType::POINT:
					collection_point_shadowed_lights.resources.push_back(ent);
					break;
				}
			}
			else {
				collection_lights.resources.push_back(ent);
			}
		}
	}
	resource_manager.SetResource(output_mesh_name, std::move(collection_meshes));
	resource_manager.SetResource(output_skeletal_mesh_name, std::move(collection_skeletal_meshes));
	resource_manager.SetResource(output_light_name, std::move(collection_lights));
	resource_manager.SetResource(output_shadowed_directional_light_name, std::move(collection_directional_shadowed_lights));
	resource_manager.SetResource(output_shadowed_point_light_name, std::move(collection_point_shadowed_lights));
}
