#include "VulkanRootSignature.h"
#include <stdexcept>
#include "VulkanRenderContext.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderDescriptorHeap.h"
#include "VulkanMaterial.h"

VulkanRootSignature::VulkanRootSignature(const RootSignatureDescriptor& descriptor_in) : parameters() , RootSignature(descriptor_in)
{
	DEFINE_VK_INSTANCE(context);
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkDescriptorSetLayoutCreateInfo set_layout = {};
	set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	
	VkDescriptorSetLayoutBinding binding = {};
	binding.descriptorCount = 1;
	binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;

	uint32_t set_id = 0;
	uint32_t binding_id = 0;
	uint32_t constant_offset = 0;
	parameters.reserve(descriptor.parameters.size());
	int global_descriptors = 1;

	for (auto& desc : descriptor.parameters) {
		auto type = VulkanUnitConverter::RootParameterTypeToDescritorType(desc.type);
		switch (desc.type)
		{
		case RootParameterType::CONSTANT_BUFFER:
		case RootParameterType::TEXTURE_2D:
		case RootParameterType::TEXTURE_2D_ARRAY:
		case RootParameterType::TEXTURE_2D_CUBEMAP:
		case RootParameterType::STORAGE_BUFFER:
			desc.binding_id = binding_id++;
			binding.descriptorType = type;
			binding.binding = desc.binding_id;
			bindings.push_back(binding);
			break;
		default:
			break;
		}
	}

	std::vector<VkDescriptorSetLayout> layouts;

	if (binding_id != 0) {
		set_id = 1;
		set_layout.bindingCount = bindings.size();
		set_layout.pBindings = bindings.data();
		set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
		vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &shader_layout);
		layouts.push_back(shader_layout);
	}

	for (auto& desc : descriptor.parameters) {
		if (desc.type == RootParameterType::MATERIAL) {
			desc.set_id = set_id++;
			desc.material_template = MaterialManager::Get()->GetMaterialTemplate(desc.name);
			layouts.push_back(std::static_pointer_cast<VulkanMaterialTemplate>(desc.material_template)->GetAllocator().GetLayout());
		}
	}


	info.setLayoutCount = layouts.size();
	info.pSetLayouts = layouts.data();

	vkCreatePipelineLayout(context->GetVkDevice(), &info, NULL, &layout);


}

VulkanRootSignature::~VulkanRootSignature()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroyPipelineLayout(context->GetVkDevice(), layout, NULL);
	vkDestroyDescriptorSetLayout(context->GetVkDevice(), shader_layout, NULL);
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