#pragma once 
#include "OpenGLRenderCommand.h"
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>
#include <string>

class OpenGLSetTexture2DCommand : public OpenGLRenderCommand {
public:
	OpenGLSetTexture2DCommand(std::shared_ptr<Pipeline> pipeline, const std::string& name, std::shared_ptr<RenderTexture2DResource> texture) : name(name), texture(texture), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderTexture2DResource> texture;
	std::shared_ptr<Pipeline> pipeline;
	std::string name;
};

class OpenGLSetTexture2DArrayCommand : public OpenGLRenderCommand {
public:
	OpenGLSetTexture2DArrayCommand(std::shared_ptr<Pipeline> pipeline, const std::string& name, std::shared_ptr<RenderTexture2DArrayResource> texture) : name(name), texture(texture), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderTexture2DArrayResource> texture;
	std::shared_ptr<Pipeline> pipeline;
	std::string name;
};

class OpenGLSetTexture2DCubemapCommand : public OpenGLRenderCommand {
public:
	OpenGLSetTexture2DCubemapCommand(std::shared_ptr<Pipeline> pipeline, const std::string& name, std::shared_ptr<RenderTexture2DCubemapResource> texture) : name(name), texture(texture), pipeline(pipeline) {}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderTexture2DCubemapResource> texture;
	std::shared_ptr<Pipeline> pipeline;
	std::string name;
};

class OpenGLGenerateMIPsCommand : public OpenGLRenderCommand {
public:
	OpenGLGenerateMIPsCommand(std::shared_ptr<RenderTexture2DResource> texture) :texture(texture){}
	virtual void Execute() override;
private:
	std::shared_ptr<RenderTexture2DResource> texture;
};