#pragma once 
#include "VulkanRenderDescriptorHeapBlock.h"
#include <vulkan/vulkan.h>
#include <Renderer/MaterialManager.h>
#include <vector>
#include <mutex>

#ifndef VK_MAX_DESCRIPTOR_FREEVECTOR_SIZE
#define VK_MAX_DESCRIPTOR_FREEVECTOR_SIZE 50
#endif

using VulkanRenderDescriptorTable = std::shared_ptr<VulkanRenderDescriptorAllocation>;


class VulkanRenderDescriptorHeap {
public:
	VulkanRenderDescriptorHeap(const std::vector<VkDescriptorPoolSize>& pool_sizes, VkDescriptorSetLayout layout);
	VulkanRenderDescriptorHeap(const VulkanRenderDescriptorHeap& ref) = delete;
	VulkanRenderDescriptorHeap& operator=(const VulkanRenderDescriptorHeap& ref) = delete;
	~VulkanRenderDescriptorHeap();

	VulkanRenderDescriptorTable Allocate();
	void FlushDescriptorDeallocations(uint32_t frame_number);

	void ReturnAllocation(VulkanRenderDescriptorAllocation* alloc);

	void DestroyAlloc(VulkanRenderDescriptorAllocation* alloc);

	const std::vector<VkDescriptorPoolSize>& GetPoolSizes() const {
		return pool_sizes;
	}

	VkDescriptorSetLayout GetLayout() const { return layout; }

private:
	VkDescriptorSetLayout layout;
	std::mutex heap_mutex;
	std::vector<std::shared_ptr<VulkanRenderDescriptorHeapBlock>> heap_blocks;

	std::vector<VkDescriptorPoolSize> pool_sizes;
	std::vector<VulkanRenderDescriptorAllocation*> free_vector;
	uint32_t current_block = 0;
}; 