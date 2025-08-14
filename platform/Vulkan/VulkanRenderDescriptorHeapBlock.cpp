#include "VulkanRenderDescriptorHeapBlock.h"
#include "VulkanRenderContext.h"
#include "VulkanRenderDescriptorHeap.h"

VulkanRenderDescriptorHeapBlock::VulkanRenderDescriptorHeapBlock(size_t size) : pool(), size(size)
{
	DEFINE_VK_INSTANCE(context);
	
	VkDescriptorPoolSize sizes[2];

	sizes[0].descriptorCount = size * 3;
	sizes[0].type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	sizes[1].descriptorCount = size * 5;
	sizes[1].type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	info.maxSets = size;
	info.poolSizeCount = 2;
	info.pPoolSizes = sizes;

	vkCreateDescriptorPool(context->GetVkDevice(), &info, NULL, &pool);

}

VulkanRenderDescriptorHeapBlock::VulkanRenderDescriptorHeapBlock(VulkanRenderDescriptorHeap* originating_heap, size_t max_sets) : originating_heap(originating_heap)
{
	DEFINE_VK_INSTANCE(context);

	std::vector<VkDescriptorPoolSize> pool_sizes;
	pool_sizes.reserve(originating_heap->GetPoolSizes().size());
	for(auto pool_size_per_set : originating_heap->GetPoolSizes()) {
		VkDescriptorPoolSize pool_size = pool_size_per_set;
		pool_size.descriptorCount *= max_sets;
		pool_sizes.push_back(pool_size);
	}

	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	info.maxSets = max_sets;
	info.poolSizeCount = pool_sizes.size();
	info.pPoolSizes = pool_sizes.data();

	vkCreateDescriptorPool(context->GetVkDevice(), &info, NULL, &pool);
}

RenderDescriptorAllocation* VulkanRenderDescriptorHeapBlock::Allocate(size_t num_of_descriptors)
{
	return nullptr;
}

RenderDescriptorAllocation* VulkanRenderDescriptorHeapBlock::Allocate(VkDescriptorSetLayout layout)
{
	DEFINE_VK_INSTANCE(context);
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorSetCount = 1;
	info.descriptorPool = pool;
	info.pSetLayouts = &layout;
	
	VulkanRenderDescriptorAllocation* alloc = new VulkanRenderDescriptorAllocation();

	auto result = vkAllocateDescriptorSets(context->GetVkDevice(), &info, &alloc->descritor_set);

	alloc->allocating_heap_block = shared_from_this();
	alloc->timeline = context->GetCurrentCpuTimelineValue();

	if (result != VK_SUCCESS) {
		delete alloc;
		return nullptr;
	}

	return alloc;
}

void VulkanRenderDescriptorHeapBlock::FlushDescriptorDeallocations(uint32_t frame_number)
{
}

VulkanRenderDescriptorHeapBlock::~VulkanRenderDescriptorHeapBlock()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroyDescriptorPool(context->GetVkDevice(), pool, NULL);
}

bool VulkanRenderDescriptorAllocation::IsInUse()
{
	DEFINE_VK_INSTANCE(context);
	return timeline > context->GetCurrentGpuTimelineValue();
}
