#pragma once 
#include "OpenGLRenderCommand.h"
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>
#include <string>

class OpenGLSetTexture2DCommand : public OpenGLRenderCommand {
public:
	OpenGLSetTexture2DCommand(Pipeline* pipeline, const std::string& name, std::shared_ptr<RenderTexture2DResource> texture) : name(name), texture(texture), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderTexture2DResource> texture;
	Pipeline* pipeline;
	std::string name;
};

class OpenGLGenerateMIPsCommand : public OpenGLRenderCommand {
public:
	OpenGLGenerateMIPsCommand(std::shared_ptr<RenderTexture2DResource> texture) :texture(texture){}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderTexture2DResource> texture;
};