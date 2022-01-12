#include "OpenGLPipelineCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLShaderManager.h>

void OpenGLSetPipelineCommand::Execute()
{
	glUseProgram(static_cast<const OpenGLShader*>(pipeline->GetShader())->GetShaderProgram());
}
