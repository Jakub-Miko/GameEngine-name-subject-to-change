#include "OpenGLFrameBufferCommands.h"
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <GL/glew.h>
#include <Profiler.h>

void OpenGLSetRenderTargetCommand::Execute()
{
	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<OpenGLRenderFrameBufferResource*>(framebuffer.get())->GetRenderId());
}

void OpenGLSetDefaultRenderTargetCommand::Execute()
{
	if (frame_buffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<OpenGLRenderFrameBufferResource*>(frame_buffer.get())->GetRenderId());
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}