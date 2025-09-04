#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDeferredDestruction.h"
#include <memory>

class VulkanRenderDescriptorHeap;
class VulkanRenderDescriptorHeapBlock;


class VulkanRenderDescriptorAllocation : public VulkanDeferredDestruction {
public:
	virtual ~VulkanRenderDescriptorAllocation() {}
	
	bool IsInUse();

	virtual bool Destroy() override;

public:
	VkDescriptorSet descritor_set;
	std::weak_ptr<VulkanRenderDescriptorHeapBlock> allocating_heap_block; // The heap to return to after freeing
	uint64_t timeline; // the last submit which used this table. if not yet passed a new set needs to be allocated
};

class VulkanRenderDescriptorHeapBlock : public std::enable_shared_from_this<VulkanRenderDescriptorHeapBlock> {
public:
	VulkanRenderDescriptorHeapBlock(size_t size);

	VulkanRenderDescriptorHeapBlock(VulkanRenderDescriptorHeap* originating_heap, size_t max_sets);

	VulkanRenderDescriptorHeapBlock(const VulkanRenderDescriptorHeapBlock& other) = delete; 

	VulkanRenderDescriptorHeapBlock(VulkanRenderDescriptorHeapBlock&& other) = delete;

	VulkanRenderDescriptorAllocation* Allocate(size_t num_of_descriptors);

	VulkanRenderDescriptorAllocation* Allocate(VkDescriptorSetLayout layout);

	uint32_t GetSize() const { return size; }

	void FlushDescriptorDeallocations(uint32_t frame_number);

	virtual ~VulkanRenderDescriptorHeapBlock();

	VulkanRenderDescriptorHeap* GetOriginatingHeap() { return originating_heap; }

	VkDescriptorPool GetPool() { return pool; }

private:
	VkDescriptorPool pool = VK_NULL_HANDLE;
	VulkanRenderDescriptorHeap* originating_heap = nullptr;
	uint32_t size = 128;
};
