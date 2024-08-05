/**
* @file EntityTypes.h
* 
* @brief This file contains hardcoded templates for individual entities
*/
#pragma once
#include <World/Components/TransformComponent.h>
#include <glm/glm.hpp>
#include <World/Entity.h>

class World;

/**
 * @brief Default entity creation template, handles registering the entity to the Scene hierarchy and assigning its TransformComponent
 * 
 * Either this or EntityType needs to be used for an entity to be made a proper part of a scene.
 * Otherwise Registering with the SceneGraph and assigning a Transform component needs to be done manually
 * @note This template is used by default with World::CreateEntity and similar methods.
*/
class EntityType : public Entity {
public:

	static void CreateEntity(World& world, Entity entity, Entity parent);
	static void CreateEntity(World& world, Entity entity, Entity parent, const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f);

};

/**
 * @brief Entity creation template used to create entities as parts of a prefab hierarchy instead of a Scene hierarchy
 * 
 * Either this or EntityType needs to be used for an entity to be made a proper part of a scene.
 * Otherwise Registering with the SceneGraph and assigning a Transform component needs to be done manually
*/
class PrefabChildEntityType : public Entity {
public:
	static void CreateEntity(World& world, Entity entity, Entity parent, bool include_transform = false);
	static void CreateEntity(World& world, Entity entity, Entity parent, const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f);
};

/**
 * @brief No longer used
*/
class SquareEntityType : public EntityType {
public:

	static void CreateEntity(World& world, Entity entity, Entity parent, glm::vec4 color, glm::vec2 position = glm::vec2(0.0f), glm::vec2 size = glm::vec2(1.0f));

};

/**
 * @brief Functions the same as EntityType but also assigns a CameraComponent
 * 
 * This is just for convenience and as an example of how to create other EntityTypes
*/
class CameraEntityType : public EntityType {
public:

	static void CreateEntity(World& world, Entity entity, Entity parent,float fov, float zNear, float zFar, float aspect_ratio);
	static void CreateEntity(World& world, Entity entity, Entity parent, float fov, float zNear, float zFar, float aspect_ratio ,const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f);

};