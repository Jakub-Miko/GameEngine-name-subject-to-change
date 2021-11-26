#pragma once
#include <World/Components/TransformComponent.h>
#include <glm/glm.hpp>
#include <World/Entity.h>

class World;

class EntityType : public Entity {
public:

	static void CreateEntity(World& world, Entity entity);

};


class SquareEntityType : public EntityType {
public:

	static void CreateEntity(World& world, Entity entity, glm::vec4 color, glm::vec2 position = glm::vec2(0.0f), glm::vec2 size = glm::vec2(1.0f));

};