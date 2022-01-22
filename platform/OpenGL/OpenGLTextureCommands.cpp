#include "OpenGLTextureCommands.h"
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/OpenGL/OpenGLPipelineManager.h>
#include <GL/glew.h>

void OpenGLSetTexture2DCommand::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline)->SetTexture2D(name, texture);
}
