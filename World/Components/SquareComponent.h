#include <glm/glm.hpp>

struct SquareComponent {
	SquareComponent(glm::vec4 color = glm::vec4(1.0f)) : color(color) {}
	glm::vec4 color;
};