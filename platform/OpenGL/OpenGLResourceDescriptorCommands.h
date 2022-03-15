#pragma once 
#include "OpenGLRenderCommand.h"
#include <Renderer/RenderDescriptorHeap.h>
#include <Renderer/PipelineManager.h>

class OpenGLSetDescriptorTableCommand : public OpenGLRenderCommand {
public:
	OpenGLSetDescriptorTableCommand(std::shared_ptr<Pipeline> pipeline, const std::string& name, RenderDescriptorTable table) : name(name), pipeline(pipeline), table(table) {}
	virtual void Execute() override;
private:
	RenderDescriptorTable table;
	std::shared_ptr<Pipeline> pipeline;
	std::string name;
};