#include "OpenGLConstantBufferCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLPipelineManager.h>

void OpenGLSetConstantBufferCommandName::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline)->SetConstantBuffer(name, buffer);
}

void OpenGLSetConstantBufferCommandId::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline)->SetConstantBuffer(binding_id, buffer);
}
