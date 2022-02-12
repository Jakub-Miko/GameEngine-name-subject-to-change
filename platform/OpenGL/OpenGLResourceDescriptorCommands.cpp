#include "OpenGLResourceDescriptorCommands.h"
#include <platform/OpenGL/OpenGLPipelineManager.h>


void OpenGLSetDescriptorTableCommand::Execute()
{
	static_cast<OpenGLPipeline*>(pipeline)->SetDescriptorTable(name, table);
}
