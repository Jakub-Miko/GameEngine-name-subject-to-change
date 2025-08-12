#include "VulkanRenderCommandList.h"
#include "VulkanRenderCommandAllocator.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderContext.h"
#include "VulkanPipelineManager.h"
#include "VulkanRenderResourceManager.h"
#include "VulkanRootSignature.h"
#include "VulkanRenderDescriptorHeapBlock.h"
#include "VulkanRenderDescriptorHeap.h"
#include <Renderer/TextureManager.h>
#include <Application.h>
#include <Window.h>

VulkanRenderCommandList::VulkanRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc) : RenderCommandList(renderer, alloc), dependency_handler(),
	current_framebuffer(nullptr), current_pipeline(nullptr)
{
	DEFINE_VK_INSTANCE(context);

	VkCommandBufferAllocateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = NULL;
	info.commandPool = *(static_cast<VkCommandPool*>(alloc->Get()));
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;

	vkAllocateCommandBuffers(context->GetVkDevice(), &info, &command_buffer);
	
	dependency_handler = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->GetDependencyHandler();
	
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(command_buffer, &begin_info);
}

VulkanRenderCommandList::~VulkanRenderCommandList()
{
	DEFINE_VK_INSTANCE(context);
	static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnDependencyHandler(dependency_handler);
	vkFreeCommandBuffers(context->GetVkDevice(), *(static_cast<VkCommandPool*>(m_Alloc->Get())), 1, &command_buffer);
}

void VulkanRenderCommandList::SetPipeline(std::shared_ptr<Pipeline> pipeline)
{
	dependency_handler->PipelineChange(this, pipeline);

	current_pipeline = std::dynamic_pointer_cast<VulkanPipeline>(pipeline);
	auto vulkan_pipeline = static_cast<VulkanPipeline*>(pipeline.get());
	auto vk_pipeline = vulkan_pipeline->GetVkPipeline();
	vkCmdBindPipeline(command_buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, *vk_pipeline);
}

void VulkanRenderCommandList::Execute()
{
}

void VulkanRenderCommandList::SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer)
{
}

void VulkanRenderCommandList::SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer)
{
	if (!current_pipeline) {
		throw std::runtime_error("Cannot set a constant buffer before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(semantic_name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::CONSTANT_BUFFER) {
		throw std::runtime_error("The parameter " + semantic_name + " is not a constant buffer.\n");
	}

	auto bind_point = param.binding_id;


	dependency_handler->AddDrawDependency(this, buffer, VulkanCommandListDependencyType::READ, RenderState::COMMON, param_id);

	auto vk_buffer_handle = std::static_pointer_cast<VulkanRenderBufferResource>(buffer)->GetBuffer();
	
	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = vk_buffer_handle;
	buffer_info.offset = 0;
	buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet update_data = {};
	update_data.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	update_data.descriptorCount = 1;
	update_data.dstSet = NULL;
	update_data.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	update_data.dstBinding = bind_point;
	update_data.pBufferInfo = &buffer_info;
	update_data.dstArrayElement = 0;

	vkCmdPushDescriptorSet_KHR(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sig->GetPipelineLayout(), 0, 1, &update_data);
}

void VulkanRenderCommandList::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture)
{
	if (!current_pipeline) {
		throw std::runtime_error("Cannot set a texture before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(semantic_name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::TEXTURE_2D) {
		throw std::runtime_error("The parameter " + semantic_name + " is not a texture.\n");
	}

	auto bind_point = param.binding_id;

	dependency_handler->AddDrawDependency(this, texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_SAMPLE, param_id);

	auto vk_image_view = std::static_pointer_cast<VulkanRenderTexture2DResource>(texture)->GetImageView();
	
	VkDescriptorImageInfo image_info = {};
	image_info.imageView = vk_image_view;
	image_info.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(texture->GetBufferDescriptor().sampler)->GetSampler();

	VkWriteDescriptorSet update_data = {};
	update_data.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	update_data.descriptorCount = 1;
	update_data.dstSet = NULL;
	update_data.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	update_data.dstBinding = bind_point;
	update_data.pImageInfo = &image_info;
	update_data.dstArrayElement = 0;

	vkCmdPushDescriptorSet_KHR(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sig->GetPipelineLayout(), 0, 1, &update_data);
}

void VulkanRenderCommandList::SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture)
{
		if (!current_pipeline) {
		throw std::runtime_error("Cannot set a texture array before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(semantic_name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::TEXTURE_2D_ARRAY) {
		throw std::runtime_error("The parameter " + semantic_name + " is not a texture array.\n");
	}

	auto bind_point = param.binding_id;

	dependency_handler->AddDrawDependency(this, texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_SAMPLE, param_id);

	auto vk_image_view = std::static_pointer_cast<VulkanRenderTexture2DArrayResource>(texture)->GetImageView();
	
	VkDescriptorImageInfo image_info = {};
	image_info.imageView = vk_image_view;
	image_info.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(texture->GetBufferDescriptor().sampler)->GetSampler();

	VkWriteDescriptorSet update_data = {};
	update_data.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	update_data.descriptorCount = 1;
	update_data.dstSet = NULL;
	update_data.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	update_data.dstBinding = bind_point;
	update_data.pImageInfo = &image_info;
	update_data.dstArrayElement = 0;

	vkCmdPushDescriptorSet_KHR(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sig->GetPipelineLayout(), 0, 1, &update_data);
}

void VulkanRenderCommandList::SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture)
{
	if (!current_pipeline) {
		throw std::runtime_error("Cannot set a texture cubemap before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(semantic_name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::TEXTURE_2D_CUBEMAP) {
		throw std::runtime_error("The parameter " + semantic_name + " is not a texture cubemap.\n");
	}

	auto bind_point = param.binding_id;

	dependency_handler->AddDrawDependency(this, texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_SAMPLE, param_id);

	auto vk_image_view = std::static_pointer_cast<VulkanRenderTexture2DCubemapResource>(texture)->GetImageView();
	
	VkDescriptorImageInfo image_info = {};
	image_info.imageView = vk_image_view;
	image_info.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(texture->GetBufferDescriptor().sampler)->GetSampler();

	VkWriteDescriptorSet update_data = {};
	update_data.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	update_data.descriptorCount = 1;
	update_data.dstSet = NULL;
	update_data.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	update_data.dstBinding = bind_point;
	update_data.pImageInfo = &image_info;
	update_data.dstArrayElement = 0;

	vkCmdPushDescriptorSet_KHR(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sig->GetPipelineLayout(), 0, 1, &update_data);
}

void VulkanRenderCommandList::SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state)
{
	dependency_handler->SetResourceDefaultState(resource, state);
}

void VulkanRenderCommandList::SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer)
{
	OutsideRenderPass(); // if a render pass was active, end it, so we can set a new framebuffer and the next rendering command will resume it
	current_framebuffer = framebuffer;
	dependency_handler->RenderTargetChange(this, framebuffer);
}

void VulkanRenderCommandList::SetDefaultRenderTarget()
{
	DEFINE_VK_INSTANCE(context);
	OutsideRenderPass(); // if a render pass was active, end it, so we can set a new framebuffer and the next rendering command will resume it
	auto frame_buffer = Renderer::Get()->GetDefaultFrameBuffer();
	if(frame_buffer) {
		current_framebuffer = frame_buffer;
	} else {
		auto buffer = Application::Get()->GetWindow()->GetRenderSurface()->GetCurrentFrameBuffer();
		current_framebuffer = buffer;
	}
}

void VulkanRenderCommandList::Clear()
{
	OutsideRenderPass();
	const auto& desc = current_framebuffer->GetBufferDescriptor();
	VkClearColorValue clear{ {0,0,0,0} };
	VkClearDepthStencilValue clear_depth { 1.0,0 };
	if (current_framebuffer) {
		for (auto color_attachment : desc.color_attachments) {
			auto vk_res = static_cast<VulkanRenderTextureResource*>(color_attachment.resource->GetExtensionData());
			dependency_handler->AddDependency(this, color_attachment.resource, VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST);
			VkImageSubresourceRange range;
			range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.layerCount = VK_REMAINING_ARRAY_LAYERS;
			range.baseMipLevel = color_attachment.level;
			range.levelCount = 1;

			vkCmdClearColorImage(command_buffer, vk_res->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

		}

		auto vk_depth_res = static_cast<VulkanRenderTextureResource*>(desc.depth_stencil_attachment.resource->GetExtensionData());
		dependency_handler->AddDependency(this, desc.depth_stencil_attachment.resource, VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST);
		VkImageSubresourceRange range;
		range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
		range.baseArrayLayer = 0;
		range.layerCount = VK_REMAINING_ARRAY_LAYERS;
		range.baseMipLevel = desc.depth_stencil_attachment.level;
		range.levelCount = 1;

		vkCmdClearDepthStencilImage(command_buffer, vk_depth_res->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depth, 1, &range);

	}
	else {
		throw std::runtime_error("No Render target was set for the clear operation.\n");
	}
}

void VulkanRenderCommandList::SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer)
{
	index_buffer = buffer;
	are_index_vertex_buffers_bound = false;
}

void VulkanRenderCommandList::SetVertexBuffer(std::shared_ptr<RenderBufferResource> buffer)
{
	vertex_buffer = buffer;
	are_index_vertex_buffers_bound = false;
}

void VulkanRenderCommandList::SetScissorRect(const RenderScissorRect& in_scissor_rect)
{
	scissor_rect = in_scissor_rect;
	is_scissorrect_defined = false;
}

void VulkanRenderCommandList::SetViewport(const RenderViewport& in_viewport)
{

	viewport = in_viewport;
	is_viewport_defined = false;
}

void VulkanRenderCommandList::SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table)
{
	throw std::runtime_error("Not implemented\n");
}

void VulkanRenderCommandList::GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture)
{
}

void VulkanRenderCommandList::Draw(uint32_t index_count, bool use_unsined_short_as_index, int index_offset)
{
	dependency_handler->FlushDrawDependencies(this);
	InsideRenderPass();
	FlushDrawState();

	if(!are_index_vertex_buffers_bound) {
		auto vk_vertex_buffer = std::static_pointer_cast<VulkanRenderBufferResource>(vertex_buffer);
		auto vk_vertex_buffer_handle = vk_vertex_buffer->GetBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer_handle, &offset);

		auto vk_index_buffer = std::static_pointer_cast<VulkanRenderBufferResource>(index_buffer);
		auto vk_index_buffer_handle = vk_index_buffer->GetBuffer();
		vkCmdBindIndexBuffer(command_buffer, vk_index_buffer_handle, 0, use_unsined_short_as_index ? VkIndexType::VK_INDEX_TYPE_UINT16 : VkIndexType::VK_INDEX_TYPE_UINT32);

		are_index_vertex_buffers_bound = true;
	}

	vkCmdDrawIndexed(command_buffer, index_count, 1,index_offset / (use_unsined_short_as_index ? 2 : 4) , 0, 0);
}

void VulkanRenderCommandList::DrawArray(uint32_t vertex_count)
{
	dependency_handler->FlushDrawDependencies(this);
	InsideRenderPass();
	FlushDrawState();

	if(!are_index_vertex_buffers_bound) {
		auto vk_vertex_buffer = std::static_pointer_cast<VulkanRenderBufferResource>(vertex_buffer);
		auto vk_vertex_buffer_handle = vk_vertex_buffer->GetBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer_handle, &offset);

		are_index_vertex_buffers_bound = true;
	}

	vkCmdDraw(command_buffer, vertex_count, 1,0,0);
}

void VulkanRenderCommandList::SetMaterial(const std::string& name, std::shared_ptr<Material> material)
{
	if (!current_pipeline) {
		throw std::runtime_error("Cannot set a material before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::MATERIAL) {
		throw std::runtime_error("The parameter " + name + " is not a material.\n");
	}

	material->UpdateValues(this);

	auto bind_point = param.set_id;

	auto desc_table = std::static_pointer_cast<VulkanRenderDescriptorAllocation>(GetMutableMaterialDescriptorTable(material.get()));


	dependency_handler->AddMaterialDependency(this, material, param_id);

	if (auto buffer = GetMaterialConstantBuffer(material.get())) {
		AddDependency(buffer, VulkanCommandListDependencyType::READ, RenderState::COMMON);
	}

	dependency_handler->AddDescriptorTableDependency(this, desc_table);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sig->GetPipelineLayout(), bind_point, 1, &desc_table->descritor_set, 0, NULL);

}

void VulkanRenderCommandList::UpdateMaterial(std::shared_ptr<Material> material) // TODO: In dire need of a revision
{
	DEFINE_VK_INSTANCE(context);
	using Material_status = Material::Material_status;
	using MaterialParameter_flags = Material::MaterialParameter_flags;
	
	bool needs_table_update = false;

	auto& parameters = GetMutableMaterialParameters(material.get());
	auto material_template = material->GetMaterialTemplate();
	auto constant_buffer = GetMaterialConstantBuffer(material.get());
	auto& descriptor_table = GetMutableMaterialDescriptorTable(material.get());
	auto& status = GetMutableMaterialStatus(material.get());

	int image_update_num = 0;
	int buffer_update_num = 0;

	for (int i = 0; i < parameters.size(); i++) {
		auto& param = parameters[i];
		auto& layout_item = material_template->GetMaterialTemplateParameters().layout_items[i];

		if (param.IsDirty()) { // TODO: Accumulating all writes first and than uploading might be a better idea.
			switch (param.type)
			{
			case MaterialLayoutItemType::INT:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, &std::get<int>(param.resource), sizeof(int), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::MAT3:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, glm::value_ptr(std::get<glm::mat3>(param.resource)), sizeof(glm::mat3), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::MAT4:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, glm::value_ptr(std::get<glm::mat4>(param.resource)), sizeof(glm::mat4), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::SCALAR:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, &std::get<float>(param.resource), sizeof(float), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::VEC2:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, glm::value_ptr(std::get<glm::vec2>(param.resource)), sizeof(glm::vec2), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::VEC3:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, glm::value_ptr(std::get<glm::vec3>(param.resource)), sizeof(glm::vec3), layout_item.constant_buffer_offset);
				break;
			case MaterialLayoutItemType::VEC4:
				RenderResourceManager::Get()->UploadDataToBuffer(this, constant_buffer, glm::value_ptr(std::get<glm::vec4>(param.resource)), sizeof(glm::vec4), layout_item.constant_buffer_offset);
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
			default:
				throw std::runtime_error("Invalid material type.\n");
			}
		}
		param.flags &= ~MaterialParameter_flags::DIRTY;
	}

	if (!needs_table_update && status != Material_status::UNINITIALIZED) return; // For buffer updates there's no need to update the descriptor table, unless we are still using the default table

	auto old_desc_table = descriptor_table;
	descriptor_table = material_template->AllocateMaterialDescriptor();

	if (constant_buffer) { 
		RenderResourceManager::Get()->CreateConstantBufferDescriptor(descriptor_table, 0, constant_buffer); // On Initialization, we need to at bind our constant buffer
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
		auto& layout_item = material_template->GetMaterialTemplateParameters().layout_items[i];
		bool is_uniform;
		auto type = VulkanUnitConverter::MaterialLayoutItemTypeToDescritorType(param.type, is_uniform);

		if (!std::holds_alternative<std::monostate>(param.resource)) {
			switch (param.type)
			{
			case MaterialLayoutItemType::TEXTURE:
				if (std::holds_alternative<std::string>(param.resource)) {
					auto path = std::get<std::string>(param.resource);
					if (path.empty()) {
						material->SetParameter(param.name, TextureManager::Get()->GetDefaultTexture());
					}
					else {
						material->SetTexture(param.name, std::get<std::string>(param.resource));
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

void VulkanRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
{
}

void VulkanRenderCommandList::DrawSquare(const glm::mat4& transform, glm::vec4 color)
{
}

VulkanCommandListDependencyState VulkanRenderCommandList::AddDependency(std::shared_ptr<RenderResource> dep_resource, VulkanCommandListDependencyType access_type, RenderState desired_state)
{
	return dependency_handler->AddDependency(this,dep_resource, access_type, desired_state);
}	

VulkanCommandListDependencyState VulkanRenderCommandList::GetDependency(std::shared_ptr<RenderResource> dep_resource)
{
	return dependency_handler->GetDependency(dep_resource);
}

void VulkanRenderCommandList::InsideRenderPass()
{
	if (!render_pass_active) {
		if (!current_framebuffer) {
			throw std::runtime_error("A valid render target needs to be set before submitting rendering commands.\n");
		}

		VkRenderingInfo info = static_cast<VulkanRenderFrameBufferResource*>(current_framebuffer.get())->GetRenderingInfo();

		vkCmdBeginRendering(command_buffer, &info);

		render_pass_active = true;
	}
}

void VulkanRenderCommandList::OutsideRenderPass()
{
	if (render_pass_active) {
		vkCmdEndRendering(command_buffer);
	}
	render_pass_active = false;
}

void VulkanRenderCommandList::FlushDrawState()
{
	if(!is_scissorrect_defined || !is_viewport_defined) {
		auto framebuf = std::static_pointer_cast<VulkanRenderFrameBufferResource>(current_framebuffer);
		auto area = framebuf->GetRenderingInfo().renderArea;
		if(!is_scissorrect_defined) {
			if(scissor_rect.size.x == 0) {
				scissor_rect.offset = {area.offset.x, area.offset.y};
				scissor_rect.size = {area.extent.width, area.extent.height};
			}
			VkRect2D scissor = {};
			scissor.extent = { (uint32_t)scissor_rect.size.x, (uint32_t)scissor_rect.size.y };
			scissor.offset = { (int32_t)scissor_rect.offset.x, (int32_t)scissor_rect.offset.y };
			vkCmdSetScissor(command_buffer, 0,1, &scissor);
			is_scissorrect_defined = true;
		}
		if(!is_viewport_defined) {
			if(viewport.min_depth == viewport.max_depth) {
				viewport.offset = {area.offset.x, area.offset.y};
				viewport.size = {area.extent.width, area.extent.height};
				viewport.min_depth = 0.0f;
				viewport.max_depth = 1.0f;
			}
			VkViewport vk_viewport = {};
			vk_viewport.width = viewport.size.x;
			vk_viewport.height = -viewport.size.y;
			vk_viewport.x = viewport.offset.x;
			vk_viewport.y = viewport.offset.y + viewport.size.y;
			vk_viewport.minDepth = viewport.min_depth;
			vk_viewport.maxDepth = viewport.max_depth;
			vkCmdSetViewport(command_buffer,0,1,&vk_viewport);
			is_viewport_defined = true;
		}
	}
}

// If this works, i'll name it the GOD FUNCTION, since basically performs most if not all implicit synchronization.
// Also in a year since writing this, only god will know whats going on here
// Scratch that, it's been a couple of months and I'm already lost.
VulkanCommandListDependencyState DefaultVulkanDependencyHandler::AddDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, 
	VulkanCommandListDependencyType access_type, RenderState desired_state ,VulkanCommandListDependencyExtra extra)
{
	auto fnd = dependencies.find(resource);
	VulkanCommandListDependencyState current_dep; 
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	bool found;
	if (found = fnd != dependencies.end() && fnd->second.type != VulkanCommandListDependencyType::NONE) { // never overwrite the expected value, only the first command matters
		current_dep = fnd->second;
		fnd->second.previous_access = access_type;
		if (access_type == VulkanCommandListDependencyType::WRITE) { // Make sure Read doesnt overwrite write
			fnd->second.type = access_type;
		}
		fnd->second.current_state = desired_state;
	} else {
		auto default_state = static_cast<VulkanRenderResource*>(resource->GetExtensionData())->GetDefaultState();
		VulkanCommandListDependencyState dep = {};
		dep.current_state = desired_state;
		dep.desired_final_state = found ? fnd->second.desired_final_state : RenderState::EMPTY; // if a previous entry that was empty was found, it was used to set the desired state
		dep.expected_state = default_state; // Write access is allowed to use uninitialized resources
		dep.previous_access = access_type;
		dep.type = access_type;
		dep.allow_uninitialized = access_type == VulkanCommandListDependencyType::WRITE;
		dependencies.insert_or_assign(resource, dep);
		current_dep = dep;
		current_dep.type = VulkanCommandListDependencyType::INVALID; // used to identify the first occurrence of a dependency which doesn't need to be synchronized
	}

	switch (resource->GetResourceType())
	{
	case RenderResourceType::RenderBufferResource:
	{
		if (current_dep.type == VulkanCommandListDependencyType::INVALID) {
			break; // First access to a buffer resource is implicityly synchronized and all memory is always visible so we dont need to do anything
		}
		
		VulkanRenderBufferResource* vk_resource = static_cast<VulkanRenderBufferResource*>(resource.get());
		if (current_dep.previous_access == VulkanCommandListDependencyType::WRITE && access_type == VulkanCommandListDependencyType::READ) { //Synchronize and Make Data available
			list->OutsideRenderPass(); // Emiting a barrier pauses a rendering pass
			manager->BufferBarrier(list, std::static_pointer_cast<RenderBufferResource>(resource), extra.source_stage, extra.target_stage, current_dep.type, access_type);
		}
		else if (current_dep.previous_access != VulkanCommandListDependencyType::READ || access_type != VulkanCommandListDependencyType::READ) { // for write after write, or write affter read, no visibility operations are required, but we must ensure ordering
			list->OutsideRenderPass(); // Emiting a barrier pauses a rendering pass
			manager->BufferBarrier(list, std::static_pointer_cast<RenderBufferResource>(resource), extra.source_stage, extra.target_stage, current_dep.type, access_type);
		}
		break;
	}
	case RenderResourceType::RenderTexture2DResource:
	case RenderResourceType::RenderTexture2DArrayResource:
	case RenderResourceType::RenderTexture2DCubemapResource: // TODO: we are needlessly emmiting a pipeline barrier when no dependency entry for a texture yet exists.
	{
		// In the first access, execution and memory_barrier are always false, but we may need to transition the image.
		RenderState source = current_dep.type == VulkanCommandListDependencyType::INVALID ? resource->GetRenderState() : current_dep.current_state;

		VulkanRenderTextureResource* vk_resource = static_cast<VulkanRenderTextureResource*>(resource->GetExtensionData());
		bool transition = source != desired_state; // if the requested type differs then change it.
		bool execution_barrier = current_dep.previous_access != VulkanCommandListDependencyType::READ || access_type != VulkanCommandListDependencyType::READ; // if the requested type differs then change it.


		if (transition || execution_barrier) {
			VkImageSubresourceRange range;
			range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(vk_resource->GetFormat()) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.baseMipLevel = 0;
			range.layerCount = VK_REMAINING_ARRAY_LAYERS;
			range.levelCount = VK_REMAINING_MIP_LEVELS;

			list->OutsideRenderPass(); // Emiting a barrier pauses a rendering pass
			manager->TransitionImage(list, vk_resource, range, source, desired_state,
				extra.source_stage, extra.target_stage, current_dep.type, access_type);
		}

		break;
	}
	default:
		throw std::runtime_error("Invalid resource type.\n");
	}

	return current_dep;
}

void DefaultVulkanDependencyHandler::SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state)
{
	auto fnd = dependencies.find(resource);
	if(fnd != dependencies.end()) {
		fnd->second.desired_final_state = state;
	} else {
		VulkanCommandListDependencyState dependency = {};
		dependency.expected_state = static_cast<VulkanRenderResource*>(resource->GetExtensionData())->GetDefaultState();
		dependency.current_state = dependency.expected_state;
		dependency.previous_access = VulkanCommandListDependencyType::NONE;
		dependency.type = VulkanCommandListDependencyType::NONE;
		dependency.desired_final_state = state;
		dependency.allow_uninitialized = false;
		dependencies.insert(std::make_pair(resource, dependency));
	}
}

void DefaultVulkanDependencyHandler::AddDrawDependency(VulkanRenderCommandList *list, std::shared_ptr<RenderResource> resource, VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id)
{
	draw_state.AddDrawDependency(list, resource, access_type, desired_state, bind_id);
}

void VulkanDrawState::AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, 
	VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id)
{
	VulkanDrawResource res;
	res.resource = resource;
	res.dependency = VulkanCommandListDependency {access_type, desired_state};
	if(BindResource(bind_id)) {
		currently_bound_count++;
	}
	pending_dependencies.push_back(res);
}

void DefaultVulkanDependencyHandler::AddMaterialDependency(VulkanRenderCommandList *list, std::shared_ptr<Material> material, uint32_t bind_id)
{
	draw_state.SetMatertialResources(list, material, bind_id);
}

void VulkanDrawState::SetMatertialResources(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t bind_id)
{

	auto& params = list->GetCurrentPipeline()->GetSignature().GetDescriptor().parameters; 
	if(params[bind_id].material_template != material->GetMaterialTemplate()) {
		throw std::runtime_error("Invalid material bound to id " + std::to_string(bind_id) + ".\n");
	}

	auto material_template = material->GetMaterialTemplate();
	auto set_id = params[bind_id].set_id;
	bool new_binding_occured = false;

	VulkanCommandListDependency dep = {};
	dep.access_type = VulkanCommandListDependencyType::READ;
	dep.desired_state = RenderState::COMMON;
	
	if(BindResource(bind_id)) { // binding 0 is reserved for the optional constant buffer holding constants
		new_binding_occured = true;
	};

	for (int i = 0; i < material->GetMaterialParameters().size(); i++) {
		std::shared_ptr<RenderResource> resource;
		auto& parameter = material->GetMaterialParameters()[i];
		auto& layout_item = material_template->GetMaterialTemplateParameters().layout_items[i];
		switch (parameter.type)
		{
		case MaterialLayoutItemType::CONSTANT_BUFFER:
			resource = std::get<std::shared_ptr<RenderBufferResource>>(parameter.resource);
			break;
		case MaterialLayoutItemType::TEXTURE:
			if (std::holds_alternative<MaterialTextureType>(parameter.resource)) {
				resource = std::get<MaterialTextureType>(parameter.resource).texture;
			}
			break;
		case MaterialLayoutItemType::TEXTURE_2D_ARRAY:
			resource = std::get<std::shared_ptr<RenderTexture2DArrayResource>>(parameter.resource);
			break;
		case MaterialLayoutItemType::TEXTURE_2D_CUBEMAP:
			resource = std::get<std::shared_ptr<RenderTexture2DCubemapResource>>(parameter.resource);
			break;
		default:
			continue;
		}
		dep.desired_state = parameter.type == MaterialLayoutItemType::CONSTANT_BUFFER ? RenderState::COMMON : RenderState::TEXTURE_SAMPLE;

		VulkanDrawResource res;
		res.resource = resource;
		res.dependency = dep;

		pending_dependencies.push_back(res);
	}

	if(material->GetConstantBuffer()) {
		dep.desired_state = RenderState::COMMON;

		VulkanDrawResource res;
		res.resource = material->GetConstantBuffer();
		res.dependency = dep;
		pending_dependencies.push_back(res);
	}

	if(new_binding_occured) {
		currently_bound_count++;
	}
}


bool VulkanDrawState::IsPipelineReady()
{
    return currently_bound_count == expected_binding_count;
}

void DefaultVulkanDependencyHandler::FlushDrawDependencies(VulkanRenderCommandList* list)
{
	if(!draw_state.IsPipelineReady()) {
		throw std::runtime_error("Pipeline Resources were not fully bound before pipeline usage.\n");
	}

	AddDependency(list, list->GetVertexBuffer(),  VulkanCommandListDependencyType::READ, RenderState::IN_USE_VERTEX_BUFFER);
	AddDependency(list, list->GetIndexBuffer(),  VulkanCommandListDependencyType::READ, RenderState::IN_USE_INDEX_BUFFER);


	for (auto& dep : draw_state.pending_dependencies) {
		AddDependency(list, dep.resource, dep.dependency.access_type, dep.dependency.desired_state);
	}

	if(framebuffer_dependency_pending) {
		auto framebuffer_desc = list->GetCurrentFrameBuffer()->GetBufferDescriptor();

		if(framebuffer_desc.depth_stencil_attachment.resource) {
			AddDependency(list, framebuffer_desc.depth_stencil_attachment.resource, VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_DEPTH_STENCIL_ATTACHMENT);
		}
		
		for(auto color_attachment : framebuffer_desc.color_attachments) {
			AddDependency(list, color_attachment.resource, VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_COLOR_ATTACHMENT);
		}

		framebuffer_dependency_pending = false;
	}

	//draw_state.pending_dependencies.clear(); //We cannot do this because resources can be updated in between draws without rebinding the material.

	if (!list->GetVertexBuffer()) {
		throw std::runtime_error("Vertex buffer was not set on the pipeline.\n");
	}

	if (!list->GetVertexBuffer()) {
		throw std::runtime_error("Index buffer was not set on the pipeline.\n");
	}

}

void DefaultVulkanDependencyHandler::AddDescriptorTableDependency(VulkanRenderCommandList *list, RenderDescriptorTable desc_table)
{
	draw_state.used_descriptor_tables.insert(desc_table);
}

void DefaultVulkanDependencyHandler::PipelineChange(VulkanRenderCommandList *list, std::shared_ptr<Pipeline> new_pipeline)
{
	auto current_pipeline = list->GetCurrentPipeline();
	// If pipeline signatures dont match invalidate the current dependencies, we compare pointers, since signatures should originate from the same layout
	if (!current_pipeline || &current_pipeline->GetSignature() != &new_pipeline->GetSignature()) {
		draw_state.InvalidateDrawDependencies(list, new_pipeline);
	}
}

void DefaultVulkanDependencyHandler::RenderTargetChange(VulkanRenderCommandList *list, std::shared_ptr<RenderFrameBufferResource> new_framebuffer)
{
	framebuffer_dependency_pending = true;
}

void VulkanDrawState::InvalidateDrawDependencies(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline)
{
	draw_resources.clear();
	pending_dependencies.clear();
	currently_bound_count = 0;
	expected_binding_count = new_pipeline->GetSignature().GetDescriptor().parameters.size();
}

bool DefaultVulkanDependencyHandler::IsPipelineReady()
{
	return draw_state.IsPipelineReady();
}

VulkanCommandListDependencyState DefaultVulkanDependencyHandler::GetDependency(std::shared_ptr<RenderResource> resource)
{
	auto fnd = dependencies.find(resource);
	if (fnd != dependencies.end()) {
		return fnd->second;
	}

	return VulkanCommandListDependencyState();
}

DefaultVulkanDependencyHandler::VulkanDependencyHandlerFeedback DefaultVulkanDependencyHandler::FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value)
{
	DEFINE_VK_INSTANCE(context);
	uint64_t timeline_requirement = 0;
	VulkanRenderResource* resource;
	VulkanRenderCommandList* vk_command_list = static_cast<VulkanRenderCommandList*>(list);
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	for (auto& dependency : dependencies) {
		resource = static_cast<VulkanRenderResource*>(dependency.first->GetExtensionData());

		if (!dependency.second.allow_uninitialized && dependency.first->GetRenderState() == RenderState::UNINITIALIZED) { // If we allow uninitialed resource then accept the resource
			throw std::runtime_error("Attempted to read an uninitialized resource or the resource changed type between command recording and command list submit.\n");
		}

		
		auto old_default_state = resource->GetDefaultState();
		auto requested_default_state = dependency.second.desired_final_state;
		auto new_state = requested_default_state == RenderState::EMPTY ? old_default_state : requested_default_state;
		
		if(old_default_state != dependency.second.expected_state) {
			throw std::runtime_error("Default state of a resource was changed before command buffer was submitted which invalidated the command buffer, check for misplaced usage of SetDefaultResouce.\n");
		}
		
		dependency.first->SetRenderState(new_state); // Change the resource back to its default state, this also serves to mark the resource initialized
		
		bool transitioned = false;

		if (dependency.first->GetExtensionData()->IsTexture()) {
			auto texture = static_cast<VulkanRenderTextureResource*>(dependency.first->GetExtensionData());
			if (new_state != dependency.second.current_state) {
				VkImageSubresourceRange range;
				range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(texture->GetFormat()) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseArrayLayer = 0;
				range.baseMipLevel = 0;
				range.layerCount = VK_REMAINING_ARRAY_LAYERS;
				range.levelCount = VK_REMAINING_MIP_LEVELS; 
				
				manager->TransitionImage(list, texture, range, dependency.second.current_state, new_state, PipelineStage::ALL_STAGES, PipelineStage::ALL_STAGES);
				transitioned = true;
			}
		}

		if(transitioned) {
			dependency.second.type = VulkanCommandListDependencyType::WRITE; // Transition counts as write
		}

		switch (dependency.second.type)
		{
		case VulkanCommandListDependencyType::READ:
			timeline_requirement = std::max(resource->write_timeline, timeline_requirement); // On read we need to wait for all writes to finish, we dont care about other reads
			resource->read_timeline = new_timeline_value;
			break;
		case VulkanCommandListDependencyType::WRITE:
			timeline_requirement = std::max(std::max(resource->write_timeline, resource->read_timeline), timeline_requirement); // On write we need to wait for reads as well
			resource->write_timeline = new_timeline_value;
			break;
		case VulkanCommandListDependencyType::NONE:
			break; // Ignore none since its only used when changing default state, and if actual layout transition occurs WRITE type is used instead
		default:
			throw std::runtime_error("Invalid dependency type.\n");
		}
	}

	for(auto table : draw_state.used_descriptor_tables) {
		auto vk_desc_table = std::static_pointer_cast<VulkanRenderDescriptorAllocation>(table);
		vk_desc_table->timeline = new_timeline_value;
	}

	VulkanDependencyHandlerFeedback feedback;
	feedback.timeline_wait = timeline_requirement;

	return feedback;
}

void DefaultVulkanDependencyHandler::Reset()
{
	dependencies.clear();
	non_dependent_resources.clear();
	draw_state = VulkanDrawState();
	framebuffer_dependency_pending = true;
}
