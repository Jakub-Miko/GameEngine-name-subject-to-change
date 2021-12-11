#include "EntityTypes.h"
#include <World/World.h>
#include <World/Components/SquareComponent.h>

void EntityType::CreateEntity(World& world, Entity entity, Entity parent)
{
	SceneNode* parent_node;
	if (parent.id == Entity().id) {
		parent_node = nullptr;
	}
	else {
		parent_node = world.GetComponentSync<TransformComponent>(parent).scene_node;
	}
	
	SceneNode* new_node = world.GetSceneGraph()->AddEntity(entity, parent_node);

	world.SetComponent<TransformComponent>(entity, TransformComponent(new_node));
}

void EntityType::CreateEntity(World& world, Entity entity, Entity parent, const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation_axis, float rotation_angle)
{
	SceneNode* parent_node;
	if (parent.id == Entity().id) {
		parent_node = nullptr;
	}
	else {
		parent_node = world.GetComponentSync<TransformComponent>(parent).scene_node;
	}

	SceneNode* new_node = world.GetSceneGraph()->AddEntity(entity, parent_node);
	
	world.SetComponent<TransformComponent>(entity, TransformComponent(new_node,translation,scale,rotation_axis,rotation_angle));
}


void SquareEntityType::CreateEntity(World& world, Entity entity, Entity parent, glm::vec4 color, glm::vec2 position, glm::vec2 size)
{
	EntityType::CreateEntity(world, entity, parent, glm::vec3(position, 0.0f), glm::vec3(size,1.0f));
	world.SetComponent<SquareComponent>(entity, SquareComponent(color));
}
