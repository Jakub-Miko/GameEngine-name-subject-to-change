#include "VulkanMaterial.h"
#include "VulkanRenderResourceManager.h"
#include "VulkanUnitConverter.h"
#include <algorithm>
#include <Renderer/TextureManager.h>

VulkanMaterialTemplate::VulkanMaterialTemplate(const MaterialLayout &layout, const std::string &name, Private dummy) : MaterialTemplate(layout, name)
{
	DEFINE_VK_INSTANCE(context);

	VkDescriptorSetLayoutCreateInfo set_layout = {};
	set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	uint32_t constant_buffer_offset = 0;
	uint32_t binding_point = 0;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	uint32_t texture_desc_count = 0;
	std::vector<VkDescriptorPoolSize> descriptor_sizes;
	
	for (auto& binding : material_parameters.layout_items) {
		bool is_uniform_value;
		VkDescriptorType type = VulkanUnitConverter::MaterialLayoutItemTypeToDescritorType(binding.type, is_uniform_value);
		if (is_uniform_value) {
			VkDescriptorSetLayoutBinding vk_binding = {};
			vk_binding.descriptorCount = 1;
			vk_binding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			vk_binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			vk_binding.binding = 0;
			bindings.push_back(vk_binding);
			descriptor_sizes.push_back(VkDescriptorPoolSize { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });
			binding_point = 1;
			break;
		}
	}
	
	for (auto& binding : material_parameters.layout_items) {
		bool is_uniform_value;
		VkDescriptorType type = VulkanUnitConverter::MaterialLayoutItemTypeToDescritorType(binding.type, is_uniform_value);
		
		if (!is_uniform_value) {
			VkDescriptorSetLayoutBinding vk_binding = {};
			vk_binding.descriptorCount = 1;
			vk_binding.descriptorType = type;
			vk_binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			vk_binding.binding = binding_point++;
			bindings.push_back(vk_binding);
			
			auto fnd = std::find_if(descriptor_sizes.begin(), descriptor_sizes.end(), [type](const VkDescriptorPoolSize& item) { return item.type == type; });

			if(fnd != descriptor_sizes.end()) {
				fnd->descriptorCount++;
			} else {
				descriptor_sizes.push_back(VkDescriptorPoolSize { type, 1 });
			}
			
			binding.set_binding = vk_binding.binding;
		}
		else {
			binding.constant_buffer_offset = constant_buffer_offset;
			constant_buffer_offset += VulkanUnitConverter::MaterialLayoutItemTypeToSize(binding.type);
		}
	}

	material_parameters.const_buffer_size = constant_buffer_offset;
	set_layout.bindingCount = bindings.size();
	set_layout.pBindings = bindings.data();

	VkDescriptorSetLayout desc_set_layout;

	vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &desc_set_layout);
	
    material_allocator = std::make_unique<VulkanRenderDescriptorHeap>(descriptor_sizes, desc_set_layout);
}

std::shared_ptr<Material> VulkanMaterialTemplate::CreateMaterial()
{
    return std::make_shared<VulkanMaterial>(shared_from_this());
}

VulkanMaterial::VulkanMaterial(std::shared_ptr<MaterialTemplate> material_template) : Material(material_template)
{
    auto default_temp = material_template->GetDefaultMaterial();
	descriptor_table = !default_temp ? nullptr : std::static_pointer_cast<VulkanMaterial>(default_temp)->GetDescriptorTable(); // Use the default values, first and on first used of set material or update create the actual table
	auto const_size = material_template->GetMaterialTemplateParameters().const_buffer_size;
	if (const_size != 0) {
		RenderBufferDescriptor desc;
		desc.buffer_size = const_size;
		desc.type = RenderBufferType::DEFAULT;
		desc.usage = RenderBufferUsage::CONSTANT_BUFFER;
		constant_buffer = RenderResourceManager::Get()->CreateBuffer(desc);
	}

}

void VulkanMaterial::UpdateValues(std::shared_ptr<RenderCommandList> command_list)
{
    DEFINE_VK_INSTANCE(context);
	using Material_status = Material::Material_status;
	using MaterialParameter_flags = Material::MaterialParameter_flags;
	
	bool needs_table_update = false;

	auto vk_material_template = std::static_pointer_cast<VulkanMaterialTemplate>(GetMaterialTemplate());
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());

	int image_update_num = 0;
	int buffer_update_num = 0;

	for (int i = 0; i < parameters.size(); i++) {
		auto& param = parameters[i];
		auto& layout_item = vk_material_template->GetMaterialTemplateParameters().layout_items[i];

		if (param.IsDirty()) { // TODO: Accumulating all writes first and than uploading might be a better idea.
			switch (param.type)
			{
			case MaterialLayoutItemType::INT:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, &std::get<int>(param.resource), sizeof(int), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::MAT3:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, glm::value_ptr(std::get<glm::mat3>(param.resource)), sizeof(glm::mat3), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::MAT4:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, glm::value_ptr(std::get<glm::mat4>(param.resource)), sizeof(glm::mat4), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::SCALAR:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, &std::get<float>(param.resource), sizeof(float), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::VEC2:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, glm::value_ptr(std::get<glm::vec2>(param.resource)), sizeof(glm::vec2), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::VEC3:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, glm::value_ptr(std::get<glm::vec3>(param.resource)), sizeof(glm::vec3), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::VEC4:
				RenderResourceManager::Get()->UploadDataToBuffer(command_list, constant_buffer, glm::value_ptr(std::get<glm::vec4>(param.resource)), sizeof(glm::vec4), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::TEXTURE:
			case MaterialLayoutItemType::TEXTURE_2D_ARRAY:
			case MaterialLayoutItemType::TEXTURE_2D_CUBEMAP:
				image_update_num++;
				needs_table_update = true;
				continue;
			case MaterialLayoutItemType::CONSTANT_BUFFER:
				buffer_update_num++;
				needs_table_update = true;
				continue;
			case MaterialLayoutItemType::STORAGE_BUFFER:
				buffer_update_num++;
				needs_table_update = true;
				continue;
			default:
				throw std::runtime_error("Invalid material type.\n");
			}
		}
		param.flags &= ~MaterialParameter_flags::DIRTY;
	}

	if (!needs_table_update && status != Material_status::UNINITIALIZED) return; // For buffer updates there's no need to update the descriptor table, unless we are still using the default table

	ResetDescriptorTable(); // Get a new uninitialized descriptor table which is prepared for writes.


	if (constant_buffer) { 
		manager->CreateConstantBufferDescriptor(descriptor_table, 0, constant_buffer); // On Initialization, we need to at bind our constant buffer
	}


	std::vector<VkWriteDescriptorSet> desc_set_writes;
	desc_set_writes.reserve(image_update_num + buffer_update_num);
	std::vector<VkDescriptorImageInfo > desc_set_image_infos;
	desc_set_image_infos.reserve(parameters.size());
	std::vector<VkDescriptorBufferInfo> desc_set_buffer_infos;
	desc_set_buffer_infos.reserve(parameters.size());

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorCount = 1;
	write.dstArrayElement = 0;
	write.dstSet = static_cast<VulkanRenderDescriptorAllocation*>(descriptor_table.get())->descritor_set;

	for (int i = 0; i < parameters.size(); i++) {
		auto& param = parameters[i];
		auto& layout_item = vk_material_template->GetMaterialTemplateParameters().layout_items[i];
		bool is_uniform;
		auto type = VulkanUnitConverter::MaterialLayoutItemTypeToDescritorType(param.type, is_uniform);

		if (!std::holds_alternative<std::monostate>(param.resource)) {
			switch (param.type)
			{
			case MaterialLayoutItemType::TEXTURE:
				if (std::holds_alternative<std::string>(param.resource)) {
					auto path = std::get<std::string>(param.resource);
					if (path.empty()) {
						SetParameter(param.name, TextureManager::Get()->GetDefaultTexture());
					}
					else {
						SetTexture(param.name, std::get<std::string>(param.resource));
					}
				}
				if (std::holds_alternative<MaterialTextureType>(param.resource)) {
					auto image = std::get<MaterialTextureType>(param.resource);
					auto vk_image = static_cast<VulkanRenderTextureResource*>(image.texture->GetExtensionData());

					VkDescriptorImageInfo image_info;
					image_info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
					image_info.imageView = vk_image->GetImageView();
					image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(vk_image->GetSampler())->GetSampler();

					desc_set_image_infos.push_back(image_info);

					write.descriptorType = type;
					write.dstBinding = layout_item.set_binding;
					write.pImageInfo = &desc_set_image_infos.back();

					desc_set_writes.push_back(write);
					param.flags &= ~MaterialParameter_flags::DIRTY;
				}
				break;
			case MaterialLayoutItemType::TEXTURE_2D_ARRAY:
			{
				auto image = std::get<std::shared_ptr<RenderTexture2DArrayResource>>(param.resource);
				auto vk_image = static_cast<VulkanRenderTextureResource*>(image->GetExtensionData());

				VkDescriptorImageInfo image_info;
				image_info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
				image_info.imageView = vk_image->GetImageView();
				image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(vk_image->GetSampler())->GetSampler();

				desc_set_image_infos.push_back(image_info);

				write.descriptorType = type;
				write.dstBinding = layout_item.set_binding;
				write.pImageInfo = &desc_set_image_infos.back();

				desc_set_writes.push_back(write);
				param.flags &= ~MaterialParameter_flags::DIRTY;
				break;
			}
			case MaterialLayoutItemType::TEXTURE_2D_CUBEMAP:
			{
				auto image = std::get<std::shared_ptr<RenderTexture2DCubemapResource>>(param.resource);
				auto vk_image = static_cast<VulkanRenderTextureResource*>(image->GetExtensionData());

				VkDescriptorImageInfo image_info;
				image_info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
				image_info.imageView = vk_image->GetImageView();
				image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(vk_image->GetSampler())->GetSampler();

				desc_set_image_infos.push_back(image_info);

				write.descriptorType = type;
				write.dstBinding = layout_item.set_binding;
				write.pImageInfo = &desc_set_image_infos.back();

				desc_set_writes.push_back(write);
				param.flags &= ~MaterialParameter_flags::DIRTY;
				break;
			}
			case MaterialLayoutItemType::CONSTANT_BUFFER:
			case MaterialLayoutItemType::STORAGE_BUFFER:
			{
				auto buffer = std::get<std::shared_ptr<RenderBufferResource>>(param.resource);
				auto vk_buffer = std::static_pointer_cast<VulkanRenderBufferResource>(std::get<std::shared_ptr<RenderBufferResource>>(param.resource));

				VkDescriptorBufferInfo buffer_info;
				buffer_info.offset = 0;
				buffer_info.range = buffer->GetBufferDescriptor().buffer_size;
				buffer_info.buffer = vk_buffer->GetBuffer();

				desc_set_buffer_infos.push_back(buffer_info);

				write.descriptorType = type;
				write.dstBinding = layout_item.set_binding;
				write.pBufferInfo = &desc_set_buffer_infos.back();

				desc_set_writes.push_back(write);
				param.flags &= ~MaterialParameter_flags::DIRTY;
				break;
			}
			default:
				break;
			}
		}
	}


	vkUpdateDescriptorSets(context->GetVkDevice(), desc_set_writes.size(), desc_set_writes.data(), 0, NULL);

	if (status == Material_status::UNINITIALIZED) {
		status = Material_status::OK;
	}

}

void VulkanMaterial::ResetDescriptorTable()
{
    descriptor_table = std::static_pointer_cast<VulkanMaterialTemplate>(GetMaterialTemplate())->GetAllocator().Allocate();
    status = Material_status::UNINITIALIZED;
}
