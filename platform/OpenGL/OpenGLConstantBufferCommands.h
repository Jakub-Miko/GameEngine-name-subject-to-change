#pragma once 
#include "OpenGLRenderCommand.h"
#include <Renderer/RootSignature.h>
#include <string>
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>

class OpenGLSetConstantBufferCommandId : public OpenGLRenderCommand {
public:
	OpenGLSetConstantBufferCommandId(Pipeline* pipeline, RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) : binding_id(binding_id), buffer(buffer), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	RootBinding binding_id;
	std::shared_ptr<RenderBufferResource> buffer;
	Pipeline* pipeline;
};

class OpenGLSetConstantBufferCommandName : public OpenGLRenderCommand {
public:
	OpenGLSetConstantBufferCommandName(Pipeline* pipeline, const std::string& name, std::shared_ptr<RenderBufferResource> buffer) : name(name), buffer(buffer), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderBufferResource> buffer;
	Pipeline* pipeline;
	std::string name;
};