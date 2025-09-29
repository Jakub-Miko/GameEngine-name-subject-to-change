#pragma once
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderResource.h>
#include <vulkan/vulkan.h>

class VulkanPipeline : public PipelineNativeExtension {
public:

	virtual VkPipeline* GetVkPipeline() {
		return &pipeline;
	};
	virtual ~VulkanPipeline();
protected:
	VulkanPipeline(VkPipeline pipeline) : pipeline(pipeline) {};
private:
	VkPipeline pipeline;
};

class VulkanGraphicsPipeline : public GraphicsPipeline, public VulkanPipeline, public::std::enable_shared_from_this<VulkanGraphicsPipeline> {
public:
	friend class VulkanPipelineManager;
	VulkanGraphicsPipeline(const GraphicsPipelineDescriptor& desc, VkPipeline pipeline);
	virtual RootBinding GetBindingId(const std::string& name) override;
	virtual std::shared_ptr<PipelineNativeExtension> GetPipelineNativeExtension() override {
		return shared_from_this();
	}

	virtual ~VulkanGraphicsPipeline();

private:
	VulkanGraphicsPipeline(GraphicsPipelineDescriptor&& desc);
};

class VulkanComputePipeline : public ComputePipeline, public VulkanPipeline, public::std::enable_shared_from_this<VulkanComputePipeline> {
public:
	friend class VulkanPipelineManager;
	VulkanComputePipeline(const ComputePipelineDescriptor& desc, VkPipeline pipeline);
	virtual RootBinding GetBindingId(const std::string& name) override;
	virtual std::shared_ptr<PipelineNativeExtension> GetPipelineNativeExtension() override {
		return shared_from_this();
	}
	virtual ~VulkanComputePipeline();

private:
	VulkanComputePipeline(ComputePipelineDescriptor&& desc);
};


class VulkanPipelineManager : public PipelineManager {
public:
	friend PipelineManager;
	virtual std::shared_ptr<Pipeline> CreatePipeline(const GraphicsPipelineDescriptor& desc) override;
	virtual std::shared_ptr<Pipeline> CreatePipeline(const ComputePipelineDescriptor& desc) override;
	
private:

	virtual ~VulkanPipelineManager() {}
	VulkanPipelineManager();

};