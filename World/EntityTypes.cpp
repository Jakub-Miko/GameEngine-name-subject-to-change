#include "EntityTypes.h"
#include <World/World.h>
#include <World/Components/SquareComponent.h>

void EntityType::CreateEntity(World& world, Entity entity) {
	world.SetComponent<TransformComponent>(entity, TransformComponent(glm::mat4(1.0f)));
}

void SquareEntityType::CreateEntity(World& world, Entity entity, glm::vec4 color, glm::vec2 position, glm::vec2 size)
{
	EntityType::CreateEntity(world, entity);
	world.SetComponent<SquareComponent>(entity, SquareComponent(position, size, color));
}
