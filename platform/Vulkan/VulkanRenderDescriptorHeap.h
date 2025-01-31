#pragma once 
#include <Renderer/RenderDescriptorHeap.h>
#include "VulkanRenderDescriptorHeapBlock.h"
#include "Renderer/Renderer3D/MaterialManager.h"
#include <Vulkan/vulkan.h>

#ifndef VK_MAX_DESCRIPTOR_FREEVECTOR_SIZE
#define VK_MAX_DESCRIPTOR_FREEVECTOR_SIZE 50
#endif

class VulkanRenderDescriptorHeap : public RenderDescriptorHeap {
public:
	VulkanRenderDescriptorHeap(MaterialLayout& layout);
	VulkanRenderDescriptorHeap(const VulkanRenderDescriptorHeap& ref) = delete;
	VulkanRenderDescriptorHeap& operator=(const VulkanRenderDescriptorHeap& ref) = delete;
	virtual ~VulkanRenderDescriptorHeap();

	virtual RenderDescriptorAllocationHandle Allocate(size_t num_of_descriptors);
	virtual void FlushDescriptorDeallocations(uint32_t frame_number);

	void ReturnAllocation(VulkanRenderDescriptorAllocation* alloc);

	void DestroyAlloc(VulkanRenderDescriptorAllocation* alloc);

	const std::vector<VkDescriptorPoolSize>& GetPoolSizes() const {
		return pool_sizes;
	}

private:
	VkDescriptorSetLayout layout;
	std::mutex heap_mutex;
	std::vector<VulkanRenderDescriptorHeapBlock> heap_blocks;
	std::vector<VkDescriptorPoolSize> pool_sizes;
	std::vector<VulkanRenderDescriptorAllocation*> free_vector;
	uint32_t current_block = 0;
}; 