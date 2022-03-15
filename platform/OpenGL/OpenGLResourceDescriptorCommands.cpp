#include "OpenGLResourceDescriptorCommands.h"
#include <platform/OpenGL/OpenGLPipelineManager.h>
#include <Profiler.h>

void OpenGLSetDescriptorTableCommand::Execute()
{
	PROFILE("Set Descriptor table");
	static_cast<OpenGLPipeline*>(pipeline.get())->SetDescriptorTable(name, table);
}
