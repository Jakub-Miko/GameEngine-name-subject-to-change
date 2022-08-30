#include "OpenGLTextureCommands.h"
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/OpenGL/OpenGLPipelineManager.h>
#include <GL/glew.h>

void OpenGLSetTexture2DCommand::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline.get())->SetTexture2D(name, texture);
}

void OpenGLGenerateMIPsCommand::Execute()
{
	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(texture.get())->GetRenderId());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLSetTexture2DArrayCommand::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline.get())->SetTexture2DArray(name, texture);
}
