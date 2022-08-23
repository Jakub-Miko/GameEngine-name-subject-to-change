#pragma once
#include <glm/glm.hpp>

struct Plane {
	Plane(glm::vec3 normal, float distance) : normal(normal), distance(distance) {}
	Plane(glm::vec3 normal, glm::vec3 point) : normal(normal), distance(glm::dot(normal, point)) {}
	Plane() : normal(glm::vec3(0.0f, 1.0f, 0.0f)), distance(0) {}

	glm::vec3 normal;
	float distance;
};

struct OrientedBoundingBox {
	OrientedBoundingBox(glm::vec3 size = glm::vec3(1.0f), glm::vec3 center = glm::vec3(0.0f), glm::mat3 rotation_matrix = glm::mat3(1.0f)) : size(size), center(center), rotation_matrix(rotation_matrix) {}

	glm::vec3 size;
	glm::vec3 center;
	glm::mat3 rotation_matrix;

};