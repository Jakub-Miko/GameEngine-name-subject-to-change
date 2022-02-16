#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>
#include <memory>
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>
#include <stdint.h>


class OpenGLImplicitDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLImplicitDrawCommand(Pipeline* pipeline, std::shared_ptr<RenderBufferResource> index_buffer,
		std::shared_ptr<RenderBufferResource> vertex_buffer, uint32_t index_count) 
		: index_buffer(index_buffer), index_count(index_count), vertex_buffer(vertex_buffer), pipeline(pipeline) {};
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> index_buffer;
	std::shared_ptr<RenderBufferResource> vertex_buffer;
	Pipeline* pipeline;
	uint32_t index_count;
};