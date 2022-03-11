#pragma once
#include <World/Components/TransformComponent.h>
#include <glm/glm.hpp>
#include <World/Entity.h>

class World;

class EntityType : public Entity {
public:

	static void CreateEntity(World& world, Entity entity, Entity parent);
	static void CreateEntity(World& world, Entity entity, Entity parent, const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f);

};


class SquareEntityType : public EntityType {
public:

	static void CreateEntity(World& world, Entity entity, Entity parent, glm::vec4 color, glm::vec2 position = glm::vec2(0.0f), glm::vec2 size = glm::vec2(1.0f));

};


class CameraEntityType : public EntityType {
public:

	static void CreateEntity(World& world, Entity entity, Entity parent,float fov, float zNear, float zFar, float aspect_ratio);
	static void CreateEntity(World& world, Entity entity, Entity parent, float fov, float zNear, float zFar, float aspect_ratio ,const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f);

};