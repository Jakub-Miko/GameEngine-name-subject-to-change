#pragma once
#include "OpenGLRenderCommand.h"
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>

class OpenGLSetVertexBufferCommand : public OpenGLRenderCommand {
public:
	OpenGLSetVertexBufferCommand(std::shared_ptr<RenderBufferResource> buffer, Pipeline* pipeline) : buffer(buffer), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> buffer;
	Pipeline* pipeline;
};