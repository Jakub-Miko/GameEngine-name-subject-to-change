#pragma once

struct Plane {
	Plane(glm::vec3 normal, float distance) : normal(normal), distance(distance) {}
	Plane(glm::vec3 normal, glm::vec3 point) : normal(normal), distance(glm::dot(normal, point)) {}
	Plane() : normal(glm::vec3(0.0f, 1.0f, 0.0f)), distance(0) {}

	glm::vec3 normal;
	float distance;
};