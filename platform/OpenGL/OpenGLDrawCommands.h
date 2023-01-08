#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>
#include <memory>
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>
#include <stdint.h>


class OpenGLImplicitDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLImplicitDrawCommand(std::shared_ptr<Pipeline> pipeline, std::shared_ptr<RenderBufferResource> index_buffer,
		std::shared_ptr<RenderBufferResource> vertex_buffer, uint32_t index_count, bool use_unsined_short_as_index = false, int index_offset = 0)
		: index_buffer(index_buffer), index_count(index_count), vertex_buffer(vertex_buffer), pipeline(pipeline), index_offset(index_offset), use_unsined_short_as_index(use_unsined_short_as_index){};
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> index_buffer;
	std::shared_ptr<RenderBufferResource> vertex_buffer;
	std::shared_ptr<Pipeline> pipeline;
	bool use_unsined_short_as_index;
	int index_offset;
	uint32_t index_count;
};

class OpenGLImplicitDrawArraysCommand : public OpenGLRenderCommand {
public:
	OpenGLImplicitDrawArraysCommand(std::shared_ptr<Pipeline> pipeline, std::shared_ptr<RenderBufferResource> vertex_buffer, uint32_t vertex_count)
		: vertex_count(vertex_count), vertex_buffer(vertex_buffer), pipeline(pipeline) {};
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> vertex_buffer;
	std::shared_ptr<Pipeline> pipeline;
	uint32_t vertex_count;
};