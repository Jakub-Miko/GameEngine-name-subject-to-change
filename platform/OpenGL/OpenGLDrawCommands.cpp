#include "OpenGLDrawCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLRenderResource.h>

void OpenGLImplicitDrawCommand::Execute()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<OpenGLRenderBufferResource*>(index_buffer.get())->GetRenderId());
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
}
