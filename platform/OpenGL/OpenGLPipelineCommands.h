#pragma once
#include "OpenGLRenderCommand.h"
#include <Renderer/PipelineManager.h>

class OpenGLSetPipelineCommand : public OpenGLRenderCommand {
public:
	OpenGLSetPipelineCommand(Pipeline* pipeline) : pipeline(pipeline) {}
	virtual void Execute() override;
private:
	Pipeline* pipeline;
};
