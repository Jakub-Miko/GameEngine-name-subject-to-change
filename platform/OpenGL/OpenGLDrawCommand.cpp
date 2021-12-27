#include "OpenGLDrawCommand.h"
#include <iostream>
#include <GL/glew.h>
#include <FileManager.h>
#include <Profiler.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <platform/OpenGL/Shaders/OpenGLShader.h>
#include <fstream>



struct QuadData {
	unsigned int vertex_array;
	OpenGLShaderTest shader;
	int transform_pos;
	int color_pos;
};

bool initialized = false;

static QuadData quad_data;

OpenGLDrawCommand::OpenGLDrawCommand(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
	:  color(color)
{
	tranform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x,pos.y,0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x,size.y, 1.0f));
}

OpenGLDrawCommand::OpenGLDrawCommand(const glm::mat4& matrix, glm::vec4 color) : tranform_matrix(matrix), color(color) 
{

}

void OpenGLDrawCommand::Execute()
{
	//PROFILE("DrawExecution");

	

	if (!initialized) {
	
		quad_data.shader = OpenGLShaderTest::LoadFromFile(
			FileManager::Get()->GetRenderApiAssetFilePath("shaders/Vertex_Shader.glsl"),
			FileManager::Get()->GetRenderApiAssetFilePath("shaders/Fragment_Shader.glsl")
		);

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

		quad_data.transform_pos= glGetUniformLocation(quad_data.shader.GetHandle(), "transform");
		quad_data.color_pos= glGetUniformLocation(quad_data.shader.GetHandle(), "in_color");

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
	
	quad_data.shader.Bind();
	glBindVertexArray(quad_data.vertex_array);

	glUniform4f(quad_data.color_pos, color.r,color.g,color.b,color.a);
	glUniformMatrix4fv(quad_data.transform_pos, 1, GL_FALSE, glm::value_ptr(tranform_matrix));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	
	quad_data.shader.Unbind();
	glBindVertexArray(0);
}
