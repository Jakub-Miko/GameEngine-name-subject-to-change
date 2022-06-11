#pragma once
#include "OpenGLRenderCommand.h"
#include <Renderer/PipelineManager.h>
#include <Renderer/RendererDefines.h>

class OpenGLSetPipelineCommand : public OpenGLRenderCommand {
public:
	OpenGLSetPipelineCommand(std::shared_ptr<Pipeline> pipeline) : pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<Pipeline> pipeline;
};

class OpenGLSetScissorRect : public OpenGLRenderCommand {
public:
	OpenGLSetScissorRect(const RenderScissorRect& rect) : rect(rect) {}
	virtual void Execute() override;
private:
	RenderScissorRect rect;
};

class OpenGLSetViewport : public OpenGLRenderCommand {
public:
	OpenGLSetViewport(const RenderViewport& viewport) : viewport(viewport) {}
	virtual void Execute() override;
private:
	RenderViewport viewport;
};