#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>
#include <memory>
#include <Renderer/RenderResource.h>


class OpenGLImplicitDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLImplicitDrawCommand(std::shared_ptr<RenderBufferResource> index_buffer) : index_buffer(index_buffer) {};
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> index_buffer;
};