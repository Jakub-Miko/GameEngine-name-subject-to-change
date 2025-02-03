#include "VulkanRenderDescriptorHeap.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderResourceManager.h"

VulkanRenderDescriptorHeap::VulkanRenderDescriptorHeap(MaterialLayout& layout_in) : layout(), heap_blocks(), heap_mutex()
{
	DEFINE_VK_INSTANCE(context);

	VkDescriptorSetLayoutCreateInfo set_layout = {};
	set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	uint32_t constant_buffer_offset = 0;
	uint32_t binding_point = 0;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	uint32_t texture_desc_count = 0;

	for (auto& binding : layout_in.layout_items) {
		bool is_uniform_value;
		VkDescriptorType type = VulkanUnitConverter::MaterialLayoutItemTypeToDescritorType(binding.type, is_uniform_value);
		if (is_uniform_value) {
			VkDescriptorSetLayoutBinding vk_binding = {};
			vk_binding.descriptorCount = 1;
			vk_binding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			vk_binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			vk_binding.binding = 0;
			bindings.push_back(vk_binding);
			binding_point = 1;
			break;
		}
	}

	for (auto& binding : layout_in.layout_items) {
		bool is_uniform_value;
		VkDescriptorType type = VulkanUnitConverter::MaterialLayoutItemTypeToDescritorType(binding.type, is_uniform_value);

		if (!is_uniform_value) {
			VkDescriptorSetLayoutBinding vk_binding = {};
			vk_binding.descriptorCount = 1;
			vk_binding.descriptorType = type;
			vk_binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			vk_binding.binding = binding_point++;
			bindings.push_back(vk_binding);

			if (type == VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				texture_desc_count++;
			}

			binding.set_binding = vk_binding.binding;
		}
		else {
			binding.constant_buffer_offset = constant_buffer_offset;
			constant_buffer_offset += VulkanUnitConverter::MaterialLayoutItemTypeToSize(binding.type);
		}
	}

	if (texture_desc_count != 0) {

		VkDescriptorPoolSize texture_size;
		texture_size.descriptorCount = texture_desc_count;
		texture_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes.push_back(texture_size);
	}

	if (constant_buffer_offset != 0) {
		VkDescriptorPoolSize buffer_size;
		buffer_size.descriptorCount = 1;
		buffer_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes.push_back(buffer_size);
	}


	layout_in.const_buffer_size = constant_buffer_offset;
	set_layout.bindingCount = bindings.size();
	set_layout.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &layout);

	heap_blocks.emplace_back(std::make_unique<VulkanRenderDescriptorHeapBlock>(this, 128));
}

VulkanRenderDescriptorHeap::~VulkanRenderDescriptorHeap()
{
	FlushDescriptorDeallocations(-1);
	DEFINE_VK_INSTANCE(context);
	vkDestroyDescriptorSetLayout(context->GetVkDevice(), layout, NULL);
}

RenderDescriptorAllocationHandle VulkanRenderDescriptorHeap::Allocate()
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
			current_block = current_block - 1 < 0 ? heap_blocks.size() - 1 : current_block - 1; //subtract and wrap-around (modulo is weird for negative numbers), we subtract so we try the biggest pools first
			if (original_attempt == current_block) { // if we came back to the original attempt, we allocate a new pool with twice the size
				heap_blocks.emplace_back(std::make_unique<VulkanRenderDescriptorHeapBlock>(this, heap_blocks.back()->GetSize() * 2)); //
				current_block = heap_blocks.size() - 1;
				alloc = (VulkanRenderDescriptorAllocation*)heap_blocks[current_block]->Allocate(layout);
			}
		}

		if (!alloc) { // At this point if we dont have a valid allocation something went wrong
			throw std::runtime_error("Descriptor allocation failed.\n");
		}
	}

	RenderDescriptorAllocationHandle handle = RenderDescriptorAllocationHandle(alloc, [](RenderDescriptorAllocation* alloc) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnDescriptorAllocation(alloc, static_cast<VulkanRenderDescriptorAllocation*>(alloc)->timeline);
		});


}

void VulkanRenderDescriptorHeap::FlushDescriptorDeallocations(uint32_t frame_number)
{
	std::lock_guard<std::mutex> lock(heap_mutex);
	DEFINE_VK_INSTANCE(context);
	for (auto alloc : free_vector) {
		vkFreeDescriptorSets(context->GetVkDevice(), alloc->allocating_heap_block->GetPool(), 1, &alloc->descritor_set);
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
	vkFreeDescriptorSets(context->GetVkDevice(), alloc->allocating_heap_block->GetPool(), 1, &alloc->descritor_set);
	delete alloc;
}
