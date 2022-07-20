#include "EntityTypes.h"
#include <World/World.h>
#include <World/Components/SquareComponent.h>
#include <World/Components/PrefabComponent.h>
#include <World/Components/CameraComponent.h>

void EntityType::CreateEntity(World& world, Entity entity, Entity parent)
{
	SceneNode* parent_node;
	if (parent.id == Entity().id) {
		parent_node = nullptr;
	}
	else {
		parent_node = world.GetSceneGraph()->GetSceneGraphNode(parent);
	}
	
	world.SetComponent<TransformComponent>(entity);

	SceneNode* new_node = world.GetSceneGraph()->AddEntity(entity, parent_node);
}

void EntityType::CreateEntity(World& world, Entity entity, Entity parent, const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation_axis, float rotation_angle)
{
	SceneNode* parent_node;
	if (parent.id == Entity().id) {
		parent_node = nullptr;
	}
	else {
		parent_node = world.GetSceneGraph()->GetSceneGraphNode(parent);
	}

	world.SetComponent<TransformComponent>(entity, TransformComponent(translation,scale,rotation_axis,rotation_angle));
	
	SceneNode* new_node = world.GetSceneGraph()->AddEntity(entity, parent_node);
}


void SquareEntityType::CreateEntity(World& world, Entity entity, Entity parent, glm::vec4 color, glm::vec2 position, glm::vec2 size)
{
	EntityType::CreateEntity(world, entity, parent, glm::vec3(position, 0.0f), glm::vec3(size,1.0f));
	world.SetComponent<SquareComponent>(entity, SquareComponent(color));
}

void CameraEntityType::CreateEntity(World& world, Entity entity, Entity parent, float fov, float zNear, float zFar, float aspect_ratio)
{
	EntityType::CreateEntity(world, entity, parent);
	world.SetComponent<CameraComponent>(entity, fov, zNear, zFar, aspect_ratio);
}

void CameraEntityType::CreateEntity(World& world, Entity entity, Entity parent, float fov, float zNear, float zFar, float aspect_ratio, const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation_axis, float rotation_angle)
{
	EntityType::CreateEntity(world, entity, parent, translation ,scale, rotation_axis, rotation_angle);
	world.SetComponent<CameraComponent>(entity, fov, zNear, zFar, aspect_ratio);
}

void PrefabChildEntityType::CreateEntity(World& world, Entity entity, Entity parent, bool include_transform)
{
	if (include_transform) {
		world.SetComponent<TransformComponent>(entity);
	}
	auto scene_graph = Application::GetWorld().GetSceneGraph();
	if (world.HasComponentSynced<PrefabComponent>(parent)) {
		Application::GetWorld().GetSceneGraph()->AddEntityToPrefabRoot(entity, scene_graph->GetSceneGraphNode(parent));
	}
	else {
		Application::GetWorld().GetSceneGraph()->AddEntity(entity, scene_graph->GetSceneGraphNode(parent));
	}
	
}

void PrefabChildEntityType::CreateEntity(World& world, Entity entity, Entity parent, const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation_axis, float rotation_angle)
{
	world.SetComponent<TransformComponent>(entity,TransformComponent(translation, scale, rotation_axis, rotation_angle));
}
