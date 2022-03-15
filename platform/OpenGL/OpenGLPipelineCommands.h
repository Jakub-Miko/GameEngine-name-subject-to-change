#pragma once
#include "OpenGLRenderCommand.h"
#include <Renderer/PipelineManager.h>

class OpenGLSetPipelineCommand : public OpenGLRenderCommand {
public:
	OpenGLSetPipelineCommand(std::shared_ptr<Pipeline> pipeline) : pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<Pipeline> pipeline;
};
