#pragma once 
#include "OpenGLRenderCommand.h"
#include <Renderer/RenderResource.h>


class OpenGLSetRenderTargetCommand : public OpenGLRenderCommand {
public:
	OpenGLSetRenderTargetCommand(std::shared_ptr<RenderFrameBufferResource> buffer) : framebuffer(buffer) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderFrameBufferResource> framebuffer;
};


class OpenGLSetDefaultRenderTargetCommand : public OpenGLRenderCommand {
public:
	OpenGLSetDefaultRenderTargetCommand() {}
	virtual void Execute() override;
};