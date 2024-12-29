#include "VulkanRootSignature.h"
#include <stdexcept>
#include "VulkanRenderContext.h"
#include "VulkanUnitConverter.h"



VulkanRootSignature::VulkanRootSignature(const RootSignatureDescriptor& descriptor) : parameters()
{
	DEFINE_VK_INSTANCE(context);
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	const VulkanRootSignature* signature = static_cast<const VulkanRootSignature*>();

	RootDescriptorTable global_table = {};
	int global_textures = 0;
	int global_textures_array = 0;
	int global_textures_cubemaps = 0;
	int global_buffers = 0;

	uint32_t table_id = 0;
	std::vector<VkDescriptorSetLayout> table_layouts;
	parameters.reserve(descriptor.parameters.size());
	for (auto desc : descriptor.parameters) {
		ExtraElementInfo info;
		info.type = desc.type;
		switch (desc.type)
		{
		case RootParameterType::CONSTANT_BUFFER:
			global_buffers++;
			break;
		case RootParameterType::TEXTURE_2D:
			global_textures++;
			break;
		case RootParameterType::TEXTURE_2D_ARRAY:
			global_textures_array++;
			break;
		case RootParameterType::TEXTURE_2D_CUBEMAP:
			global_textures_cubemaps++;
			break;
		default:
			break;
		}
	}

	if (global_textures > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D, global_textures, "global_textures"));

	if (global_textures_array > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D_ARRAY, global_textures_array, "global_texture_arrays"));

	if (global_textures_cubemaps > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D_CUBEMAP, global_textures_cubemaps, "global_cubemaps"));

	if (global_buffers > 0) {}
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::CONSTANT_BUFFER, global_buffers, "global_buffers"));

	// offset each range so their indicies are disjoint
	int global_buffers = global_textures + global_textures_array + global_textures_cubemaps;
	int global_textures_cubemaps = global_textures + global_textures_array;
	int global_textures_array = global_textures;
	int global_textures = 0;

	for (auto desc : descriptor.parameters) {
		ExtraElementInfo info;
		info.type = desc.type;
		switch (desc.type)
		{
		case RootParameterType::CONSTANT_BUFFER:
			info.table_binding = 0;
			info.table_index = global_buffers++;
			parameters.insert(std::make_pair(desc.name, info));
			break;
		case RootParameterType::TEXTURE_2D:
			info.table_binding = 0;
			info.table_index = global_textures++;
			parameters.insert(std::make_pair(desc.name, info));
			break;
		case RootParameterType::TEXTURE_2D_ARRAY:
			info.table_binding = 0;
			info.table_index = global_textures_array++;
			parameters.insert(std::make_pair(desc.name, info));
			break;
		case RootParameterType::TEXTURE_2D_CUBEMAP:
			info.table_binding = 0;
			info.table_index = global_textures_cubemaps++;
			parameters.insert(std::make_pair(desc.name, info));
			break;
		case RootParameterType::DESCRIPTOR_TABLE:
			table_layouts.push_back(CreateDescriptorTableParams(desc.table, table_id++, desc.name));
			break;
		default:
			break;
		}
	}



	if (global_table.size() > 0) {
		table_layouts.push_back(CreateDescriptorTableParams(global_table, table_id++, "global"));
	}
	info.setLayoutCount = table_layouts.size();
	info.pSetLayouts = table_layouts.data();

	VkPipelineLayout layout;
	vkCreatePipelineLayout(context->GetVkDevice(), &info, NULL, &layout);


}

VkDescriptorSetLayout VulkanRootSignature::CreateDescriptorTableParams(const RootDescriptorTable& table, uint32_t table_id, const std::string& name)
{
	DEFINE_VK_INSTANCE(context);
	VkDescriptorSetLayout layout;

	VkDescriptorSetLayoutCreateInfo set_layout = {};
	set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.reserve(table.size());
	int range_binding_id = 0;
	for (auto& binding : table) {
		VkDescriptorSetLayoutBinding vk_binding = {};
		vk_binding.descriptorCount = binding.size;
		vk_binding.descriptorType = VulkanUnitConverter::DescriptorTypeToVkDescriptorType(binding.type);
		vk_binding.binding = range_binding_id++;
		bindings.push_back(vk_binding);
	}

	set_layout.bindingCount = table.size();
	set_layout.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &layout);



	ExtraElementInfo info;
	info.type = RootParameterType::DESCRIPTOR_TABLE;
	info.table_binding = table_id;
	parameters.insert(std::make_pair(name, info));

	for (auto range : table) {
		binding_id++;
		if (!range.individual_names.empty()) {
			uint32_t table_index = 0;
			if (range.type == RootDescriptorType::CONSTANT_BUFFER) {
				for (auto& name : range.individual_names) {
					ExtraElementInfo info;
					info.type = RootParameterType::CONSTANT_BUFFER;
					info.table_binding = table_id;
					info.table_range = binding_id;
					info.table_array_index = table_index++;
					parameters.insert(std::make_pair(name, info));
				}
			}
			else if (range.type == RootDescriptorType::TEXTURE_2D) {
				for (auto& name : range.individual_names) {
					ExtraElementInfo info;
					info.type = RootParameterType::TEXTURE_2D;
					info.table_binding = table_id;
					info.table_range = binding_id;
					info.table_array_index = table_index++;
					parameters.insert(std::make_pair(name, info));
				}
				binding_id += range.size;
			}
			else if (range.type == RootDescriptorType::TEXTURE_2D_ARRAY) {
				for (auto& name : range.individual_names) {
					ExtraElementInfo info;
					info.type = RootParameterType::TEXTURE_2D_ARRAY;
					info.table_binding = table_id;
					info.table_range = binding_id;
					info.table_array_index = table_index++;
					parameters.insert(std::make_pair(name, info));
				}
				binding_id += range.size;
			}
			else if (range.type == RootDescriptorType::TEXTURE_2D_CUBEMAP) {
				for (auto& name : range.individual_names) {
					ExtraElementInfo info;
					info.type = RootParameterType::TEXTURE_2D_CUBEMAP;
					info.table_binding = table_id;
					info.table_range = binding_id;
					info.table_array_index = table_index++;
					parameters.insert(std::make_pair(name, info));
				}
				binding_id += range.size;
			}
		}
	}

}

int VulkanRootSignature::GetUniformBlockBindingId(const std::string& name) const
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		if (fnd->second.type == RootParameterType::CONSTANT_BUFFER) {
			return fnd->second.constant_binding_id;
		}
		else {
			throw std::runtime_error("Parameter isn't a constant buffer");
		}
	}
	else {
		throw std::runtime_error("Uniformblock with name " + name + " doesn't exist");
	}
}

int VulkanRootSignature::GetTextureSlot(const std::string& name) const
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		if (fnd->second.type == RootParameterType::TEXTURE_2D || fnd->second.type == RootParameterType::TEXTURE_2D_ARRAY || fnd->second.type == RootParameterType::TEXTURE_2D_CUBEMAP) {
			return fnd->second.texture_slot;
		}
		else {
			throw std::runtime_error("Parameter isn't a Texture 2D");
		}
	}
	else {
		throw std::runtime_error("Texture 2D with name " + name + " doesn't exist");
	}
}

uint32_t VulkanRootSignature::GetTableBinding(const std::string& name) const
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		if (fnd->second.type == RootParameterType::DESCRIPTOR_TABLE) {
			return *descriptor_tables[fnd->second.table_descriptor_id];
		}
		else {
			throw std::runtime_error("Parameter isn't a Descriptor Table");
		}
	}
	else {
		throw std::runtime_error("Descriptor Table with name " + name + " doesn't exist");
	}
}
