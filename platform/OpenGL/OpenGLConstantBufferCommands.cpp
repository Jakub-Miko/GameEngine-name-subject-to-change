#include "OpenGLConstantBufferCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLPipelineManager.h>

void OpenGLSetConstantBufferCommandName::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline.get())->SetConstantBuffer(name, buffer);
}

void OpenGLSetConstantBufferCommandId::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline.get())->SetConstantBuffer(binding_id, buffer);
}
