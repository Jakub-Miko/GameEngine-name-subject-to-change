#include <glm/glm.hpp>
#include <Core/RuntimeTag.h>

struct SquareComponent {
	RuntimeTag("SquareComponent")
	SquareComponent(glm::vec4 color = glm::vec4(1.0f)) : color(color) {}
	glm::vec4 color;
};