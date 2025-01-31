#pragma once
#include <Renderer/RenderDescriptorHeapBlock.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Vulkan/vulkan.h>

class VulkanRenderDescriptorHeap;
class VulkanRenderDescriptorHeapBlock;


class VulkanRenderDescriptorAllocation : public RenderDescriptorAllocation {
public:
	virtual ~VulkanRenderDescriptorAllocation() {}
	
public:
	VkDescriptorSet descritor_set;
	VulkanRenderDescriptorHeapBlock* allocating_heap_block; // The heap to return to after freeing
	uint64_t timeline; // the last submit which used this table. if not yet passed a new set needs to be allocated
};

class VulkanRenderDescriptorHeapBlock : public RenderDescriptorHeapBlock {
public:
	VulkanRenderDescriptorHeapBlock(size_t size);

	VulkanRenderDescriptorHeapBlock(VulkanRenderDescriptorHeap* originating_heap, size_t max_sets);

	VulkanRenderDescriptorHeapBlock(const VulkanRenderDescriptorHeapBlock& other) = delete; 

	VulkanRenderDescriptorHeapBlock(VulkanRenderDescriptorHeapBlock&& other) noexcept; //alloc for used with std::vector to allow reallocation 

	virtual RenderDescriptorAllocation* Allocate(size_t num_of_descriptors) override;

	RenderDescriptorAllocation* Allocate(VkDescriptorSetLayout layout);

	uint32_t GetSize() const { return size; }

	virtual void FlushDescriptorDeallocations(uint32_t frame_number) override;

	virtual ~VulkanRenderDescriptorHeapBlock();

	VulkanRenderDescriptorHeap* GetOriginatingHeap() { return originating_heap; }

	VkDescriptorPool GetPool() { return pool; }

private:
	VkDescriptorPool pool = VK_NULL_HANDLE;
	VulkanRenderDescriptorHeap* originating_heap = nullptr;
	uint32_t size = 128;
};
