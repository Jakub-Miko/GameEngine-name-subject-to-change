#include "OpenGLFrameBufferCommands.h"
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <GL/glew.h>

void OpenGLSetRenderTargetCommand::Execute()
{
	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<OpenGLRenderFrameBufferResource*>(framebuffer.get())->GetRenderId());
}
