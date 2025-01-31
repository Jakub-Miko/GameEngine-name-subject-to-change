#pragma once
#include <Renderer/RenderDescriptorHeapBlock.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Vulkan/vulkan.h>

class VulkanRenderDescriptorAllocation : public RenderDescriptorAllocation {
public:
	virtual ~VulkanRenderDescriptorAllocation() {}
	
public:
	VkDescriptorSet descritor_set;
};

class VulkanRenderDescriptorHeapBlock : public RenderDescriptorHeapBlock {
public:
	VulkanRenderDescriptorHeapBlock(size_t size);

	VulkanRenderDescriptorHeapBlock(const std::vector<VkDescriptorPoolSize>& pool_sizes, size_t max_sets);

	VulkanRenderDescriptorHeapBlock(const VulkanRenderDescriptorHeapBlock& other) = delete; 

	VulkanRenderDescriptorHeapBlock(VulkanRenderDescriptorHeapBlock&& other) noexcept; //alloc for used with std::vector to allow reallocation 

	virtual RenderDescriptorAllocation* Allocate(size_t num_of_descriptors) override;

	RenderDescriptorAllocation* Allocate(VkDescriptorSetLayout layout);

	virtual void FlushDescriptorDeallocations(uint32_t frame_number) override;

	virtual ~VulkanRenderDescriptorHeapBlock();

private:
	VkDescriptorPool pool = VK_NULL_HANDLE;
};
