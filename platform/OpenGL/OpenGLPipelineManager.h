#pragma once
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderDescriptorHeap.h>

class OpenGLPipeline : public Pipeline {
public:
	friend class OpenGLPipelineManager;
	virtual RootBinding GetBindingId(const std::string& name) override;
	virtual ~OpenGLPipeline() {}

	void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer);
	void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer);
	void SetTexture2D(RootBinding binding_id, std::shared_ptr<RenderTexture2DResource> buffer);
	void SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> buffer);
	
	void SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table);

private:
	OpenGLPipeline(const PipelineDescriptor& desc);
	OpenGLPipeline(PipelineDescriptor&& desc);
private:


};


class OpenGLPipelineManager : public PipelineManager {
public:
	friend PipelineManager;
	virtual std::shared_ptr<Pipeline> CreatePipeline(const PipelineDescriptor& desc) override;
	
private:
	virtual ~OpenGLPipelineManager() {}
	OpenGLPipelineManager();

};