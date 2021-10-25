#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>
#include <glm/glm.hpp>



class OpenGLDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLDrawCommand(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	virtual void Execute() override;
private:
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec4 color;
};