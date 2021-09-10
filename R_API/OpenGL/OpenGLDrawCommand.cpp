#include "OpenGLDrawCommand.h"
#include <iostream>
#include <GL/glew.h>
#include <Utility/Profiler.h>

OpenGLDrawCommand::OpenGLDrawCommand(float pos_x, float pos_y, float size_x, float size_y)
	: pos_x(pos_x), pos_y(pos_y), size_x(size_x), size_y(size_y)
{
	
}

OpenGLDrawCommand::QuadData OpenGLDrawCommand::quad_data;

bool OpenGLDrawCommand::initialized = false;

void OpenGLDrawCommand::Execute()
{
	PROFILE("DrawExecution");

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

	glBindVertexArray(quad_data.vertex_array);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
