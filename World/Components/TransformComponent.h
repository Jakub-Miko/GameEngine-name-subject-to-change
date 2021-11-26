#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

struct TransformComponent {
	TransformComponent() : TransformMatrix(1.0f) {}

	TransformComponent(const glm::mat4& mat) : TransformMatrix(mat) {}

	TransformComponent(const glm::vec3& translation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f),
		const glm::vec3& rotation_axis = glm::vec3(0,1,0), float rotation_angle = 0.0f) : TransformMatrix(1.0f)
	{
		TransformMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::scale(glm::mat4(1.0f), scale) * glm::rotate(glm::mat4(1.0f), rotation_angle, rotation_axis);
	}

	glm::mat4 TransformMatrix;
};