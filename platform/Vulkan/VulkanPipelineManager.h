#pragma once
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderDescriptorHeap.h>
#include <vulkan/vulkan.h>

class VulkanPipeline : public Pipeline {
public:
	friend class VulkanPipelineManager;
	virtual RootBinding GetBindingId(const std::string& name) override;
	virtual ~VulkanPipeline();

	VkPipeline* GetVkPipeline() { return &pipeline; }

private:
	VulkanPipeline(const PipelineDescriptor& desc, VkPipeline pipeline);
	VulkanPipeline(PipelineDescriptor&& desc);
private:
	VkPipeline pipeline;
};


class VulkanPipelineManager : public PipelineManager {
public:
	friend PipelineManager;
	virtual std::shared_ptr<Pipeline> CreatePipeline(const PipelineDescriptor& desc) override;
	
private:

	virtual ~VulkanPipelineManager() {}
	VulkanPipelineManager();

};