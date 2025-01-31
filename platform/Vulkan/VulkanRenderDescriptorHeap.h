#pragma once 
#include <Renderer/RenderDescriptorHeap.h>
#include "VulkanRenderDescriptorHeapBlock.h"
#include "Renderer/Renderer3D/MaterialManager.h"
#include <Vulkan/vulkan.h>

class VulkanRenderDescriptorHeap : public RenderDescriptorHeap {
public:
	VulkanRenderDescriptorHeap(MaterialLayout& layout);
	VulkanRenderDescriptorHeap(const VulkanRenderDescriptorHeap& ref) = delete;
	VulkanRenderDescriptorHeap& operator=(const VulkanRenderDescriptorHeap& ref) = delete;
	virtual ~VulkanRenderDescriptorHeap();

	virtual RenderDescriptorAllocationHandle Allocate(size_t num_of_descriptors);
	virtual void FlushDescriptorDeallocations(uint32_t frame_number);

private:
	VkDescriptorSetLayout layout;
	std::vector<VulkanRenderDescriptorHeapBlock> heap_blocks;
	std::vector<VkDescriptorPoolSize> pool_sizes;
	uint32_t current_block = 0;
	VulkanRenderDescriptorAllocation* free_list = nullptr;
}; 