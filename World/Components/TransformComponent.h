#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <Core/UnitConverter.h>
#include <glm/gtx/quaternion.hpp>
#include <World/SceneGraph.h>

struct TransformComponent {

	TransformComponent(SceneNode* ptr) : scene_node(ptr), TransformMatrix(1.0f), translation(0.0f), size(1.0f), rotation(glm::angleAxis(glm::degrees(0.0f), glm::vec3(0.0f,1.0f,0.0f))) {}

	TransformComponent(const TransformComponent& ref) 
		: TransformMatrix(1.0f), translation(ref.translation), size(ref.size), rotation(ref.rotation), scene_node(ref.scene_node), props(ref.props) {}


	TransformComponent(SceneNode* ptr, const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0, 1, 0), float rotation_angle = 0.0f) 
		: TransformMatrix(1.0f), translation(translation), size(scale),
		rotation(glm::angleAxis(glm::degrees(rotation_angle), glm::vec3(rotation_axis) )), scene_node(ptr)
	{

	}

	void MakeDynamic() {
		props.mode = props.mode | EntityMode::DYNAMIC;
	}

	void MakeSerializable() {
		props.mode = props.mode | EntityMode::SERIALIZABLE;
	}

	glm::vec3 translation;
	SceneNode* scene_node = nullptr;
	glm::vec3 size;
	glm::quat rotation;
	EntityProperties props = EntityProperties();
	glm::mat4 TransformMatrix;
	
};

JSON_SERIALIZABLE(TransformComponent, translation, rotation, size, props)