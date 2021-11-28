#include "EntityTypes.h"
#include <World/World.h>
#include <World/Components/SquareComponent.h>

void EntityType::CreateEntity(World& world, Entity entity)
{
	world.SetComponent<TransformComponent>(entity, TransformComponent(glm::mat4(1.0f)));
}

void EntityType::CreateEntity(World& world, Entity entity, const glm::mat4& transform) {
	world.SetComponent<TransformComponent>(entity, TransformComponent(transform));
}

void EntityType::CreateEntity(World& world, Entity entity, const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation_axis, float rotation_angle)
{
	world.SetComponent<TransformComponent>(entity, TransformComponent(translation,scale,rotation_axis,rotation_angle));
}


void SquareEntityType::CreateEntity(World& world, Entity entity, glm::vec4 color, glm::vec2 position, glm::vec2 size)
{
	EntityType::CreateEntity(world, entity, glm::vec3(position, 0.0f), glm::vec3(size,1.0f));
	world.SetComponent<SquareComponent>(entity, SquareComponent(color));
}
