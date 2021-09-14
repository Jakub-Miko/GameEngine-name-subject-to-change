#include "OpenGLDrawCommand.h"
#include <iostream>
#include <GL/glew.h>
#include <Utility/Profiler.h>
#include <platform/OpenGL/Shaders/OpenGLShader.h>
#include <fstream>


OpenGLDrawCommand::OpenGLDrawCommand(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
	: pos(pos), size(size), color(color)
{
	
}

OpenGLDrawCommand::QuadData OpenGLDrawCommand::quad_data;

bool OpenGLDrawCommand::initialized = false;

void OpenGLDrawCommand::Execute()
{
	PROFILE("DrawExecution");

	static OpenGLShader shader = OpenGLShader::LoadFromFile(
		"C:/Users/mainm/Desktop/GameEngine/PseudoCode/assets/shaders/OpenGL/Vertex_Shader.glsl",
		"C:/Users/mainm/Desktop/GameEngine/PseudoCode/assets/shaders/OpenGL/Fragment_Shader.glsl"
	);
	

	if (!initialized) {
		float quad[4 * 2] =
		{
			-0.5f,-0.5f,
			0.5f,0.5f,
			-0.5f,0.5f,
			0.5f,-0.5f
		};

		unsigned int index_buf[2 * 3] = {
			0,1,2,
			0,3,1
		};

		
		glGenVertexArrays(1, &quad_data.vertex_array);
		glBindVertexArray(quad_data.vertex_array);

		unsigned int vertex_buffer;
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, &quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, (void*)0);

		unsigned int index_buffer;
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, &index_buf, GL_STATIC_DRAW);
		glBindVertexArray(0);
		initialized = true;
	}
	int location_pos = glGetUniformLocation(shader.GetHandle(), "position");
	int location_size = glGetUniformLocation(shader.GetHandle(), "size");
	int location_color = glGetUniformLocation(shader.GetHandle(), "in_color");
	shader.Bind();
	glUniform2f(location_pos, pos.x, pos.y);
	glUniform2f(location_size, size.x, size.y);
	glUniform4f(location_color, color.r,color.g,color.b,color.a);
	glBindVertexArray(quad_data.vertex_array);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	shader.Unbind();
	glBindVertexArray(0);
}
