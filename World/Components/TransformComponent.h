#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <Core/UnitConverter.h>
#include <glm/gtx/quaternion.hpp>
#include <World/SceneGraph.h>
#include <Core/RuntimeTag.h>

struct TransformComponent {
	RuntimeTag("TransformComponent")
	TransformComponent(EntityProperties props = EntityProperties()) : TransformMatrix(1.0f), translation(0.0f), size(1.0f), rotation(glm::angleAxis(glm::degrees(0.0f), glm::vec3(0.0f, 1.0f, 0.0f))), props(props) {}

	TransformComponent(const TransformComponent& ref) 
		: TransformMatrix(1.0f), translation(ref.translation), size(ref.size), rotation(ref.rotation), props(ref.props) {}


	TransformComponent(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f, EntityProperties props = EntityProperties())
		: TransformMatrix(1.0f), translation(translation), size(scale),
		rotation(glm::angleAxis(glm::degrees(rotation_angle), glm::vec3(rotation_axis) )), props(props)
	{

	}

	glm::vec3 translation;
	glm::vec3 size;
	glm::quat rotation;
	EntityProperties props = EntityProperties();
	glm::mat4 TransformMatrix;
	
};

JSON_SERIALIZABLE(TransformComponent, translation, rotation, size, props)