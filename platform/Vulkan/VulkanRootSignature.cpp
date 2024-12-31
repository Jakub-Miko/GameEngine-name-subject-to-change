#include "VulkanRootSignature.h"
#include <stdexcept>
#include "VulkanRenderContext.h"
#include "VulkanUnitConverter.h"



VulkanRootSignature::VulkanRootSignature(const RootSignatureDescriptor& descriptor) : parameters()
{
	DEFINE_VK_INSTANCE(context);
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	RootDescriptorTable global_table = {};

	uint32_t table_id = 0;
	parameters.reserve(descriptor.parameters.size());
	table_layouts.push_back(VkDescriptorSetLayout());
	int global_descriptors = 1;

	for (auto desc : descriptor.parameters) {
		switch (desc.type)
		{
		case RootParameterType::CONSTANT_BUFFER:
			global_table.push_back(RootDescriptorTableRange(RootDescriptorType::CONSTANT_BUFFER, 1, desc.name));
			break;
		case RootParameterType::TEXTURE_2D:
			global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D, 1, desc.name));
			break;
		case RootParameterType::TEXTURE_2D_ARRAY:
			global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D_ARRAY, 1, desc.name));
			break;
		case RootParameterType::TEXTURE_2D_CUBEMAP:
			global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D_CUBEMAP, 1, desc.name));
			break;
		case RootParameterType::DESCRIPTOR_TABLE:
			table_layouts.push_back(CreateDescriptorTableParams(desc.table, table_id++, desc.name));
			break;
		default:
			break;
		}
	}



	table_layouts[0] = CreateDescriptorTableParams(global_table, 0, "global");
	info.setLayoutCount = table_layouts.size();
	info.pSetLayouts = table_layouts.data();

	vkCreatePipelineLayout(context->GetVkDevice(), &info, NULL, &layout);


}

VulkanRootSignature::~VulkanRootSignature()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroyPipelineLayout(context->GetVkDevice(), layout, NULL);
	for (auto& set_layout : table_layouts) {
		vkDestroyDescriptorSetLayout(context->GetVkDevice(), set_layout, NULL);
	}
}

VkDescriptorSetLayout VulkanRootSignature::CreateDescriptorTableParams(const RootDescriptorTable& table, uint32_t table_id, const std::string& name)
{
	DEFINE_VK_INSTANCE(context);
	VkDescriptorSetLayout layout;

	VkDescriptorSetLayoutCreateInfo set_layout = {};
	set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	VulkanDescriptorBinding info;
	info.type = RootParameterType::DESCRIPTOR_TABLE;
	info.table_binding = table_id;
	parameters.insert(std::make_pair(name, info));

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.reserve(table.size());
	int range_binding_id = 0;
	for (auto& binding : table) {
		VkDescriptorType type = VulkanUnitConverter::DescriptorTypeToVkDescriptorType(binding.type);
		RootParameterType root_type = VulkanUnitConverter::DescriptorTypeToVkRootParameterType(binding.type);
		if (binding.individual_names.empty()) {

			VkDescriptorSetLayoutBinding vk_binding = {};
			vk_binding.descriptorCount = binding.size;
			vk_binding.descriptorType = type;
			vk_binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			vk_binding.binding = range_binding_id++;
			bindings.push_back(vk_binding);

			VulkanDescriptorBinding item_info;
			item_info.type = root_type;
			item_info.table_binding = table_id;
			item_info.table_index = vk_binding.binding;
			parameters.insert(std::make_pair(binding.name, item_info));
		}
		else {
			int array_index = 0;
			for (auto& desc : binding.individual_names) {
				VkDescriptorSetLayoutBinding vk_binding = {};
				vk_binding.descriptorCount = 1;
				vk_binding.descriptorType = type;
				vk_binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
				vk_binding.binding = range_binding_id++;
				bindings.push_back(vk_binding);

				VulkanDescriptorBinding item_info;
				item_info.type = root_type;
				item_info.table_binding = table_id;
				item_info.table_index = vk_binding.binding;
				item_info.array_index = array_index++;
				parameters.insert(std::make_pair(desc, item_info));
			}
		}
	}

	set_layout.bindingCount = bindings.size();
	set_layout.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &layout);

	return layout;
}

VulkanDescriptorBinding VulkanRootSignature::GetDescriptorBinding(const std::string& name) const
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		return fnd->second;
	}
	else {
		throw std::runtime_error("DescriptorBinding with name " + name + " doesn't exist");
	}
}