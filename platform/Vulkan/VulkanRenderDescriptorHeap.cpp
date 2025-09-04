#include "VulkanRenderDescriptorHeap.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderResourceManager.h"

VulkanRenderDescriptorHeap::VulkanRenderDescriptorHeap(const std::vector<VkDescriptorPoolSize>& pool_sizes, VkDescriptorSetLayout layout) : layout(layout), heap_blocks(), heap_mutex(), current_block(0)
{
	DEFINE_VK_INSTANCE(context);
	heap_blocks.emplace_back(std::make_unique<VulkanRenderDescriptorHeapBlock>(this, 128));
}

VulkanRenderDescriptorHeap::~VulkanRenderDescriptorHeap()
{
	FlushDescriptorDeallocations(-1);
	DEFINE_VK_INSTANCE(context);
	vkDestroyDescriptorSetLayout(context->GetVkDevice(), layout, NULL);
}

VulkanRenderDescriptorTable VulkanRenderDescriptorHeap::Allocate()
{
	std::lock_guard<std::mutex> lock(heap_mutex);
	auto original_attempt = current_block;
	VulkanRenderDescriptorAllocation* alloc;
	if (!free_vector.empty()) {
		alloc = free_vector.back();
		free_vector.pop_back();
	}
	else {
		while (!(alloc = (VulkanRenderDescriptorAllocation*)heap_blocks[current_block]->Allocate(layout))) {
			current_block = current_block == 0 ? heap_blocks.size() - 1 : current_block - 1; //subtract and wrap-around (modulo is weird for negative numbers), we subtract so we try the biggest pools first
			if (original_attempt == current_block) { // if we came back to the original attempt, we allocate a new pool with twice the size
				heap_blocks.emplace_back(std::make_shared<VulkanRenderDescriptorHeapBlock>(this, heap_blocks.back()->GetSize() * 2)); //
				current_block = heap_blocks.size() - 1;
			}
		}

		if (!alloc) { // At this point if we dont have a valid allocation something went wrong
			throw std::runtime_error("Descriptor allocation failed.\n");
		}
	}

	VulkanRenderDescriptorTable handle = VulkanRenderDescriptorTable(alloc, [](VulkanRenderDescriptorAllocation* alloc) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->AddToDeferredDestructionQueue(alloc, static_cast<VulkanRenderDescriptorAllocation*>(alloc)->timeline);
		});

	return handle;
}

void VulkanRenderDescriptorHeap::FlushDescriptorDeallocations(uint32_t frame_number)
{
	std::lock_guard<std::mutex> lock(heap_mutex);
	DEFINE_VK_INSTANCE(context);
	for (auto alloc : free_vector) {
		if(auto block = alloc->allocating_heap_block.lock()) { // Take into account that the block might already have been destroyed and the descriptors are thus already freed and invalid
			vkFreeDescriptorSets(context->GetVkDevice(), block->GetPool(), 1, &alloc->descritor_set);
		}
		delete alloc;
	}
}

void VulkanRenderDescriptorHeap::ReturnAllocation(VulkanRenderDescriptorAllocation* alloc)
{
	std::lock_guard<std::mutex> lock(heap_mutex);
	if (free_vector.size() <= VK_MAX_DESCRIPTOR_FREEVECTOR_SIZE) {
		free_vector.push_back(alloc);
	}
	else {
		DestroyAlloc(alloc);
	}

}

void VulkanRenderDescriptorHeap::DestroyAlloc(VulkanRenderDescriptorAllocation* alloc)
{
	DEFINE_VK_INSTANCE(context);
	if(auto block = alloc->allocating_heap_block.lock()) { // Take into account that the block might already have been destroyed and the descriptors are thus already freed and invalid
		vkFreeDescriptorSets(context->GetVkDevice(), block->GetPool(), 1, &alloc->descritor_set);
	}
	delete alloc;
}
