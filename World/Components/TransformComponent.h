#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

struct TransformComponent {
	TransformComponent() : TransformMatrix(1.0f) {}

	TransformComponent(const glm::mat4& mat) : TransformMatrix(mat) {}

	TransformComponent(const glm::vec3& translation, const glm::vec3& scale,const glm::vec3& rotation) : TransformMatrix(1.0f)
	{
		TransformMatrix *= glm::translate(glm::mat4(1.0f, translation)) * glm::scale(glm::mat4(1.0f), scale) * glm::rotate(glm::mat4(1.0f), glm::quat(rotation));
	}

	glm::mat4 TransformMatrix;
};