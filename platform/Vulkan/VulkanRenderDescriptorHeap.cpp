#include "VulkanRenderDescriptorHeap.h"
#include "VulkanUnitConverter.h"

VulkanRenderDescriptorHeap::VulkanRenderDescriptorHeap(MaterialLayout& layout_in) : layout(), heap_blocks()
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

	VkDescriptorPoolSize texture_size;
	texture_size.descriptorCount = texture_desc_count;
	texture_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes.push_back(texture_size);

	if (constant_buffer_offset != 0) {
		VkDescriptorPoolSize buffer_size;
		texture_size.descriptorCount = 1;
		texture_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes.push_back(buffer_size);
	}


	layout_in.const_buffer_size = constant_buffer_offset;
	set_layout.bindingCount = bindings.size();
	set_layout.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &layout);

	heap_blocks.emplace_back(VulkanRenderDescriptorHeapBlock(pool_sizes));
}

VulkanRenderDescriptorHeap::~VulkanRenderDescriptorHeap()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroyDescriptorSetLayout(context->GetVkDevice(), layout, NULL);
}

RenderDescriptorAllocationHandle VulkanRenderDescriptorHeap::Allocate(size_t num_of_descriptors)
{
	


}

void VulkanRenderDescriptorHeap::FlushDescriptorDeallocations(uint32_t frame_number)
{
}
