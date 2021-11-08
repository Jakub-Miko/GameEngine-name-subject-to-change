#include <glm/glm.hpp>

struct SquareComponent {
	SquareComponent(glm::vec2 pos, glm::vec2 size, glm::vec4 color = glm::vec4(1.0f)) : pos(pos),size(size),color(color) {}
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec4 color;
};