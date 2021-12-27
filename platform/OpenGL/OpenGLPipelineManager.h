#pragma once
#include <Renderer/PipelineManager.h>

class OpenGLPipeline : public Pipeline {
public:

	virtual ~OpenGLPipeline() = default;
};

class OpenGLPipelineManager : public PipelineManager {
public:
	friend PipelineManager;
	virtual std::shared_ptr<Pipeline> CreatePipeline(const PipelineDescriptor& desc) override;
private:
	OpenGLPipelineManager();
	virtual ~OpenGLPipelineManager() {}
};