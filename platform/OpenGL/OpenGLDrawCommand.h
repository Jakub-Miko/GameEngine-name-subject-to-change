#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>
#include <glm/glm.hpp>



class OpenGLDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLDrawCommand(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	OpenGLDrawCommand(const glm::mat4& matrix, glm::vec4 color = { 1.f,1.f,1.f,1.f });

	virtual void Execute() override;
private:
	glm::mat4 tranform_matrix;
	glm::vec4 color;
};