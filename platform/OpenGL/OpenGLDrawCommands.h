#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>
#include <memory>
#include <Renderer/RenderResource.h>
#include <stdint.h>


class OpenGLImplicitDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLImplicitDrawCommand(std::shared_ptr<RenderBufferResource> index_buffer, uint32_t index_count) : index_buffer(index_buffer), index_count(index_count) {};
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> index_buffer;
	uint32_t index_count;
};