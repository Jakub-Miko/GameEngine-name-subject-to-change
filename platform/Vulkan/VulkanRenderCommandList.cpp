#include "VulkanRenderCommandList.h"
#include "VulkanMaterial.h"
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

#include "VulkanRenderResourceStore.h"

VulkanDependencyHandler::individual_resource_map_t::iterator VulkanDependencyHandler::GetNewIndividualResourceRecord(std::shared_ptr<RenderResource> resource) {
	auto fnd = individual_resource_dependencies_map.find(resource);
	if (fnd == individual_resource_dependencies_map.end()) {
		individual_resource_dependency_storage.push_back({});
		auto index = individual_resource_dependency_storage.size() - 1;
		auto& record = individual_resource_dependency_storage[index];
		auto default_state = static_cast<VulkanRenderResource*>(resource->GetExtensionData())->GetDefaultState();
		record.resource = resource;
		record.state.current_state = default_state;
		record.state.desired_final_state = RenderState::EMPTY; //Assume default desired state.
		record.state.expected_state = default_state;
		record.state.previous_access = VulkanCommandListDependencyType::NONE;
		record.state.type = VulkanCommandListDependencyType::NONE;
		record.state.allow_uninitialized = false;
		return individual_resource_dependencies_map.insert_or_assign(resource, index).first;
	} else {
		return fnd;
	}
}

VulkanRenderCommandList::VulkanRenderCommandList(std::shared_ptr<VulkanRenderCommandAllocator> alloc) : allocator(alloc)
{
	DEFINE_VK_INSTANCE(context);

	VkCommandBufferAllocateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = NULL;
	info.commandPool = alloc->GetCommandPool();
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;

	vkAllocateCommandBuffers(context->GetVkDevice(), &info, &command_buffer);
	
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(command_buffer, &begin_info);
}

VulkanRenderCommandList::~VulkanRenderCommandList()
{
	DEFINE_VK_INSTANCE(context);
	if(auto alloc = allocator.lock()) { // If the allocator no longer exists, it means 
		vkFreeCommandBuffers(context->GetVkDevice(), alloc->GetCommandPool(), 1, &command_buffer);
	}
}

void VulkanRenderCommandList::SetPipeline(std::shared_ptr<Pipeline> pipeline)
{
	dependency_handler.PipelineChange(this, pipeline);

	current_pipeline = pipeline;
	auto vulkan_pipeline = std::static_pointer_cast<VulkanPipeline>(pipeline->GetPipelineNativeExtension());
	auto vk_pipeline = vulkan_pipeline->GetVkPipeline();
	vkCmdBindPipeline(command_buffer, vulkan_pipeline->GetBindPoint(), *vk_pipeline);
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


	dependency_handler.AddDrawDependency(this, buffer, VulkanCommandListDependencyType::READ, RenderState::COMMON, param_id);

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

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdPushDescriptorSet_KHR(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), 0, 1, &update_data);
}

void VulkanRenderCommandList::SetStorageBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) {
	if (!current_pipeline) {
		throw std::runtime_error("Cannot set a storage buffer before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(semantic_name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::STORAGE_BUFFER) {
		throw std::runtime_error("The parameter " + semantic_name + " is not a storage buffer.\n");
	}

	auto bind_point = param.binding_id;


	dependency_handler.AddDrawDependency(this, buffer, VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ, RenderState::COMMON, param_id);

	auto vk_buffer_handle = std::static_pointer_cast<VulkanRenderBufferResource>(buffer)->GetBuffer();

	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = vk_buffer_handle;
	buffer_info.offset = 0;
	buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet update_data = {};
	update_data.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	update_data.descriptorCount = 1;
	update_data.dstSet = NULL;
	update_data.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	update_data.dstBinding = bind_point;
	update_data.pBufferInfo = &buffer_info;
	update_data.dstArrayElement = 0;

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdPushDescriptorSet_KHR(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), 0, 1, &update_data);
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

	dependency_handler.AddDrawDependency(this, texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_SAMPLE, param_id);

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

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdPushDescriptorSet_KHR(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), 0, 1, &update_data);
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

	dependency_handler.AddDrawDependency(this, texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_SAMPLE, param_id);

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

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdPushDescriptorSet_KHR(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), 0, 1, &update_data);
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

	dependency_handler.AddDrawDependency(this, texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_SAMPLE, param_id);

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

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdPushDescriptorSet_KHR(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), 0, 1, &update_data);
}

void VulkanRenderCommandList::SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state)
{
	dependency_handler.SetResourceDefaultState(resource, state);
}

void VulkanRenderCommandList::SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer)
{
	OutsideRenderPass(); // if a render pass was active, end it, so we can set a new framebuffer and the next rendering command will resume it
	current_framebuffer = framebuffer;
	dependency_handler.RenderTargetChange(this, framebuffer);
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
			dependency_handler.AddDependency(this, color_attachment.resource, { VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST});
			VkImageSubresourceRange range;
			range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.layerCount = VK_REMAINING_ARRAY_LAYERS;
			range.baseMipLevel = color_attachment.level;
			range.levelCount = 1;

			vkCmdClearColorImage(command_buffer, vk_res->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

		}

		auto vk_depth_res = static_cast<VulkanRenderTextureResource*>(desc.depth_stencil_attachment.resource->GetExtensionData());
		dependency_handler.AddDependency(this, desc.depth_stencil_attachment.resource, {VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST});
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

void VulkanRenderCommandList::GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture)
{
}

void VulkanRenderCommandList::Draw(uint32_t index_count, bool use_unsined_short_as_index, int index_offset)
{
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

void VulkanRenderCommandList::Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z) {
	OutsideRenderPass();
	dependency_handler.FlushDrawDependencies(this);
	vkCmdDispatch(command_buffer, thread_group_count_x, thread_group_count_y, thread_group_count_z);
}

void VulkanRenderCommandList::SetMaterial(const std::string& name, std::shared_ptr<Material> material)
{
	if (!current_pipeline) {
		throw std::runtime_error("Cannot set a material before a pipeline was bound.\n");
	}

	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto vk_mat = std::static_pointer_cast<VulkanMaterial>(material);
	auto param_id = sig->GetRootParameterId(name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::MATERIAL) {
		throw std::runtime_error("The parameter " + name + " is not a material.\n");
	}

	material->UpdateValues(SharedFromThis());

	auto bind_point = param.set_id;

	auto desc_table = vk_mat->GetDescriptorTable();


	dependency_handler.AddMaterialDependency(this, material, param_id);

	// This shouldn't be here since not constant buffer operations are performed on the setting of material.
	// if (auto buffer = vk_mat->GetConstantBuffer()) {
	// 	AddDependency(buffer, VulkanCommandListDependencyType::READ, RenderState::COMMON);
	// }

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdBindDescriptorSets(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), bind_point, 1, &desc_table->descritor_set, 0, NULL);

}

void VulkanRenderCommandList::SetResourceStore(const std::string& name,
	std::shared_ptr<RenderResourceStore> resource_store) {
	auto sig = static_cast<const VulkanRootSignature*>(&current_pipeline->GetSignature());
	auto param_id = sig->GetRootParameterId(name).parameter_id;
	auto param = sig->GetDescriptor().parameters[param_id];
	if (param.type != RootParameterType::RESOURCE_STORE) {
		throw std::runtime_error("The parameter " + name + " is not a resource store.\n");
	}
	dependency_handler.AddStoreUsage(this,resource_store, param_id);

	auto desc_set = std::static_pointer_cast<VulkanRenderResourceStore>(resource_store)->GetDescriptorSet();

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(current_pipeline->GetPipelineNativeExtension())->GetBindPoint();

	vkCmdBindDescriptorSets(command_buffer, pipeline_bind_point, sig->GetPipelineLayout(), param.set_id, 1, &desc_set, 0, NULL);
}

void VulkanRenderCommandList::AttachResourceToStoreAfterSubmission(std::shared_ptr<RenderResourceStore> store,
	std::shared_ptr<RenderResource> resource) {
	dependency_handler.EnsureInitialization(resource);
	AddSubmissionCallback([resource, store] () {
		store->AttachResource(resource);
	});
}

void VulkanRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
{
}

void VulkanRenderCommandList::DrawSquare(const glm::mat4& transform, glm::vec4 color)
{
}

void VulkanRenderCommandList::AddSubmissionCallback(std::function<void()> callback) {
	submission_callbacks.push_back(callback);
}

void VulkanRenderCommandList::AddDependency(std::shared_ptr<RenderResource> dep_resource, VulkanCommandListDependencyType access_type, RenderState desired_state)
{
	dependency_handler.AddDependency(this,dep_resource, {access_type, desired_state});
}	

VulkanCommandListDependencyState VulkanRenderCommandList::GetDependency(std::shared_ptr<RenderResource> dep_resource)
{
	return dependency_handler.GetDependency(dep_resource);
}

void VulkanRenderCommandList::InsideRenderPass()
{
	if (!render_pass_active) {
		render_pass_counter++;
	}

	if(!IsRenderPassActive() || dependency_handler.IsDrawStateDirty()) {
		dependency_handler.FlushDrawDependencies(this);
	}

	if(!render_pass_active) {
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

bool VulkanRenderCommandList::Destroy()
{
    if(auto ptr = std::static_pointer_cast<VulkanRenderCommandAllocator>(allocator.lock())) {
		ResetState();
		ptr->ReturnCommandList(this);
		return false;
	} else {
		return true;
	}
}

VulkanDependencyState VulkanDependencyHandler::UpdateDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
	const VulkanDependencyState& dependency_target_state) {

	auto fnd = GetNewIndividualResourceRecord(resource);

	auto& dependency = individual_resource_dependency_storage[fnd->second];

	VulkanDependencyState current_dep;
	if(!dependency.IsInitialized()) { // if dependency was not yet recorded or empty, initialize it.
		current_dep = {dependency.state.previous_access, dependency.resource->GetRenderState() };
		dependency.state.current_state = dependency_target_state.desired_state;
		dependency.state.previous_access = dependency_target_state.access_type;
		dependency.state.type = dependency_target_state.access_type;
		dependency.state.allow_uninitialized = (dependency_target_state.access_type & VulkanCommandListDependencyType::WRITE) != VulkanCommandListDependencyType::NONE;
		current_dep.access_type = VulkanCommandListDependencyType::NONE; // used to identify the first occurrence of a dependency which doesn't need to be synchronized

		if(resource->GetResourceStore()) {
			auto store = resource->GetResourceStore();
			auto fnd_store = resource_store_dependencies_map.find(store);
			if(fnd_store == resource_store_dependencies_map.end()) {
				dependency.store_it =
					resource_store_dependencies_map.insert_or_assign(store,RenderResourceStoreDependency()).first;
			} else {
				dependency.store_it = fnd_store;
			}
		}
	} else {
		current_dep = {dependency.state.previous_access, dependency.state.current_state };
		dependency.state.previous_access = dependency_target_state.access_type;
		dependency.state.type |= dependency_target_state.access_type;
		dependency.state.current_state = dependency_target_state.desired_state;
	}

	if(dependency.store_it.has_value()) {
		auto store = dependency.store_it.value();
		if(store->second.store_version > dependency.store_version && store->second.store_version != 0) {
			//if the last usage of the store happened after the last individual dependency, then use the state from the store operation.
			current_dep.access_type = store->first->IsReadOnly() ? VulkanCommandListDependencyType::READ : VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ;
			current_dep.desired_state = store->first->GetDescriptor().default_image_resource_state;
		}
		AddIndividualResourceOverride(fnd->second); // add the resource as override.
	}

	dependency.state.last_render_pass_used = list->GetRenderPassCounter();

	return current_dep;
}

void VulkanDependencyHandler::PrepareDependencyForEmission(VulkanRenderCommandList* list,
	std::shared_ptr<RenderResource> resource, VulkanDependencyState dependency_target_state) {

	auto fnd = GetNewIndividualResourceRecord(resource);

	auto& dependency = individual_resource_dependency_storage[fnd->second];

	bool force_emission = dependency.state.last_render_pass_used != list->GetRenderPassCounter();

	VulkanDependencyState current_dep;
	if(!dependency.IsInitialized()) { // if dependency was not yet recorded or empty, initialize it.
		current_dep = {dependency.state.previous_access, dependency.resource->GetRenderState() };
		force_emission |= current_dep.desired_state != dependency_target_state.desired_state; // we always need to emit a dependency on layout change
		current_dep.access_type = VulkanCommandListDependencyType::NONE; // used to identify the first occurrence of a dependency which doesn't need to be synchronized

		if(resource->GetResourceStore()) {
			auto store = resource->GetResourceStore();
			auto fnd_store = resource_store_dependencies_map.find(store);
			if(fnd_store == resource_store_dependencies_map.end()) {
				dependency.store_it =
					resource_store_dependencies_map.insert_or_assign(store,RenderResourceStoreDependency()).first;
			} else {
				dependency.store_it = fnd_store;
			}
		}
	} else {
		current_dep = {dependency.state.previous_access, dependency.state.current_state };
		force_emission |= current_dep.desired_state != dependency_target_state.desired_state; // we always need to emit a dependency on layout change
	}

	if(dependency.store_it.has_value()) {
		auto store = dependency.store_it.value();
		if(store->second.store_version > dependency.store_version && store->second.store_version != 0) {
			//if the last usage of the store happened after the last individual dependency, then use the state from the store operation.
			current_dep.access_type = store->first->IsReadOnly() ? VulkanCommandListDependencyType::READ : VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ;
			current_dep.desired_state = store->first->GetDescriptor().default_image_resource_state;
		}
	}

	barrier.AddBarrier(resource, current_dep, dependency_target_state);
	barrier.force_emission |= force_emission;
	barrier.dependency_updates.push_back({fnd->second, dependency_target_state});
}

void VulkanDependencyHandler::PrepareStoreDependencyForEmission(VulkanRenderCommandList* list,
	std::shared_ptr<RenderResourceStore> store) {

	auto fnd = resource_store_dependencies_map.find(store);


	if(fnd == resource_store_dependencies_map.end()) {
		RenderResourceStoreDependency dep = {};
		dep.store_version = 0;
		dep.first_resource_override = -1;
		fnd = resource_store_dependencies_map.insert_or_assign(store,dep).first;
	}

	auto& dep = fnd->second;

	bool forced_emission = dep.last_render_pass_used != list->GetRenderPassCounter();

	uint32_t override_index = dep.first_resource_override;
	while(override_index != -1) {
		auto& override = individual_resource_dependency_storage[override_index];
		auto usage = store->IsReadOnly() ? VulkanCommandListDependencyType::READ : VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ;
		PrepareDependencyForEmission(list, override.resource, {usage, store->GetDescriptor().default_image_resource_state});
		override_index = override.next_resource;
	}

	if(dep.store_version != 0 && !store->IsReadOnly()) {
		auto access_type = store->IsReadOnly() ? VulkanCommandListDependencyType::READ : VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ;
		auto state = VulkanDependencyState {access_type, RenderState::COMMON, PipelineStage::ALL_STAGES};
		barrier.AddGlobalMemoryBarrier(state, state);
	}

	dep.first_resource_override = -1;
	dep.store_version++;

	barrier.force_emission |= forced_emission;
}

void VulkanDependencyHandler::FlushPreparedDependencies(VulkanRenderCommandList* list) {
	bool inside_pass = list->IsRenderPassActive();
	bool defer_emission = inside_pass && !barrier.force_emission;
	if(!defer_emission) {
		if(inside_pass) {
			list->OutsideRenderPass();
			list->IncrementRenderPassCounter();
		}
		auto barrier_info = barrier.GetBarrierInfo();
		vkCmdPipelineBarrier2(*list->GetVkCommandBuffer(), &barrier_info);
	}

	for(auto& dep : barrier.dependency_updates) {
		auto& dependency = individual_resource_dependency_storage[dep.dependency_index];
		if(!dependency.IsInitialized()) { // if dependency was not yet recorded or empty, initialize it.
			dependency.state.current_state = dep.target_state.desired_state;
			dependency.state.previous_access = dep.target_state.access_type;
			dependency.state.type = dep.target_state.access_type;
			dependency.state.allow_uninitialized = (dep.target_state.access_type & VulkanCommandListDependencyType::WRITE) != VulkanCommandListDependencyType::NONE;

			if(dependency.resource->GetResourceStore()) {
				auto store = dependency.resource->GetResourceStore();
				auto fnd_store = resource_store_dependencies_map.find(store);
				if(fnd_store == resource_store_dependencies_map.end()) {
					dependency.store_it =
						resource_store_dependencies_map.insert_or_assign(store,RenderResourceStoreDependency()).first;
				} else {
					dependency.store_it = fnd_store;
				}
			}
		} else {
			if(!defer_emission) {
				dependency.state.previous_access = dep.target_state.access_type;
			} else { // if we won't emit a dependency, we need to accumulate the access.
				dependency.state.previous_access |= dep.target_state.access_type;
			}
			dependency.state.type |= dep.target_state.access_type;
			dependency.state.current_state = dep.target_state.desired_state;
		}

		if(dependency.store_it.has_value()) {
			AddIndividualResourceOverride(dep.dependency_index); // add the resource as override.
		}

		dependency.state.last_render_pass_used = list->GetRenderPassCounter();
	}

	barrier.ClearBarriers();
}

// If this works, i'll name it the GOD FUNCTION, since basically performs most if not all implicit synchronization.
// Also in a year since writing this, only god will know whats going on here
// Scratch that, it's been a couple of months and I'm already lost.
void VulkanDependencyHandler::AddDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
	const VulkanDependencyState& dependency_target_state)
{
	if(list->IsRenderPassActive()) {
		throw std::runtime_error("Dependencies cannot be added after a render pass has started through AddDependency.\n");
	}
	barrier.ClearBarriers();
	PrepareDependencyForEmission(list, resource, dependency_target_state);
	FlushPreparedDependencies(list);
}

void VulkanDependencyHandler::AddDependencyToBarrier(VulkanRenderCommandList* list,
                                                     std::shared_ptr<RenderResource> resource, VulkanDependencyState dependency_target_state, VulkanBarrier& barrier) {
	auto current_state = UpdateDependency(list, resource, dependency_target_state);
	barrier.AddBarrier(resource, current_state, dependency_target_state);
}

void VulkanDependencyHandler::AddStoreUsage(VulkanRenderCommandList* list, std::shared_ptr<RenderResourceStore> store, uint32_t bind_id) {
	draw_state.AddStoreUsage(list, store, bind_id);
}

void VulkanDependencyHandler::IterateStoreDependencies(std::shared_ptr<RenderResourceStore> store,
	std::function<void(const VulkanDrawResource&)> dependency_callback, bool clear_after_iteration) {
	auto fnd = resource_store_dependencies_map.find(store);
	if(fnd == resource_store_dependencies_map.end()) {
		return;
	}
	auto& dep = fnd->second;
	uint32_t override_index = dep.first_resource_override;
	while(override_index != -1) {
		auto& override = individual_resource_dependency_storage[override_index];
		auto usage = store->IsReadOnly() ? VulkanCommandListDependencyType::READ : VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ;
		VulkanDrawResource resource = { override.resource, usage, override.state.expected_state };
		dependency_callback(resource);
		override_index = override.next_resource;
	}

	if(clear_after_iteration) {
		dep.first_resource_override = -1;
	}
}

void VulkanDependencyHandler::ClearStoreDependencies(std::shared_ptr<RenderResourceStore> store) {
	auto fnd = resource_store_dependencies_map.find(store);
	if(fnd != resource_store_dependencies_map.end()) {
		fnd->second.first_resource_override = -1;
	}
}

void VulkanDependencyHandler::SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state)
{
	auto fnd = individual_resource_dependencies_map.find(resource);
	if(resource->GetResourceStore()) {
		throw std::runtime_error("Cannot set default state for a resource that is part of a store.\n");
	}
	if(fnd == individual_resource_dependencies_map.end()) {
		auto dependency = GetNewIndividualResourceRecord(resource);
		auto& dependency_state = individual_resource_dependency_storage[dependency->second].state;
		dependency_state.desired_final_state = state;
		dependency_state.allow_uninitialized = true;
	} else {
		individual_resource_dependency_storage[fnd->second].state.desired_final_state = state;
	}
}

void VulkanDependencyHandler::AddDrawDependency(VulkanRenderCommandList *list, std::shared_ptr<RenderResource> resource, VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id)
{
	draw_state.AddDrawDependency(list, resource, access_type, desired_state, bind_id);
}

void VulkanDrawState::AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, 
	VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id)
{
	VulkanDrawResource res;
	res.resource = resource;
	res.dependency_target_state = {access_type, desired_state};

	AssertIndividualResourceValidity(resource);

	auto fnd = draw_resources.find(bind_id);
	if(fnd == draw_resources.end()) {
		pending_dependencies.push_back(res);
		draw_resources.insert_or_assign(bind_id, pending_dependencies.size() - 1);
		currently_bound_count++;
	} else {
		pending_dependencies[fnd->second] = res;
	}

	dirty = true;
}

void VulkanDrawState::AddStoreUsage(VulkanRenderCommandList* list, std::shared_ptr<RenderResourceStore> store,
	uint32_t bind_id) {
	for(auto& resource : pending_dependencies) {
		if(resource.resource->GetResourceStore() == store) {
			throw std::runtime_error("Individual resources belonging to a resource store and the store itself cannot be used in the same pipeline.\n");
		}
	}

	auto fnd = draw_resources.find(bind_id);
	if(fnd == draw_resources.end()) {
		pending_stores.push_back(store);
		currently_bound_count++;
	} else {
		pending_stores[fnd->second] = store;
	}

	dirty = true;
}

void VulkanDependencyHandler::AddMaterialDependency(VulkanRenderCommandList *list, std::shared_ptr<Material> material, uint32_t bind_id)
{
	draw_state.SetMaterialResources(list, material, bind_id);
}

VkDependencyInfo VulkanBarrier::GetBarrierInfo() const {
	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.memoryBarrierCount = global_memory_barriers.size();
	info.pMemoryBarriers = global_memory_barriers.data();
	info.bufferMemoryBarrierCount = buffer_barriers.size();
	info.pBufferMemoryBarriers = buffer_barriers.data();
	info.imageMemoryBarrierCount = image_barriers.size();
	info.pImageMemoryBarriers = image_barriers.data();
	return info;
}

void VulkanBarrier::AddBarrier(std::shared_ptr<RenderResource> resource,
	const VulkanDependencyState& source_dependency_state, const VulkanDependencyState& target_dependency_state) {

	if (source_dependency_state.access_type == VulkanCommandListDependencyType::INVALID) {
		return; // First access to a buffer/image resource is implicitly synchronized and all memory is always visible so we dont need to do anything
	}

	switch (resource->GetResourceType())
	{
	case RenderResourceType::RenderBufferResource:
	{

		if (source_dependency_state.access_type != VulkanCommandListDependencyType::READ || target_dependency_state.access_type != VulkanCommandListDependencyType::READ) { // for write after write, or write affter read, no visibility operations are required, but we must ensure ordering
			auto vk_buffer = std::static_pointer_cast<VulkanRenderBufferResource>(resource)->GetBuffer();

			VkBufferMemoryBarrier2 barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
			barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(source_dependency_state.stage);
			barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(target_dependency_state.stage);
			barrier.srcAccessMask = VulkanUnitConverter::DependencyToVkAccess(source_dependency_state.access_type);
			barrier.dstAccessMask = VulkanUnitConverter::DependencyToVkAccess(target_dependency_state.access_type);
			barrier.buffer = vk_buffer;
			barrier.size = VK_WHOLE_SIZE;

			buffer_barriers.push_back(barrier);
		}
		break;
	}
	case RenderResourceType::RenderTexture2DResource:
	case RenderResourceType::RenderTexture2DArrayResource:
	case RenderResourceType::RenderTexture2DCubemapResource: // TODO: we are needlessly emitting a pipeline barrier when no dependency entry for a texture yet exists.
	{
		RenderState source = source_dependency_state.desired_state;

		VulkanRenderTextureResource* vk_resource = static_cast<VulkanRenderTextureResource*>(resource->GetExtensionData());
		bool transition = source != target_dependency_state.desired_state; // if the requested type differs then change it.
		bool execution_barrier = source_dependency_state.access_type != VulkanCommandListDependencyType::READ || target_dependency_state.access_type != VulkanCommandListDependencyType::READ; // if the requested type differs then change it.

		if (transition || execution_barrier) {
			auto vk_image = static_cast<VulkanRenderTextureResource*>(resource->GetExtensionData())->GetImage();
			VkImageSubresourceRange range;
			range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(vk_resource->GetFormat()) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.baseMipLevel = 0;
			range.layerCount = VK_REMAINING_ARRAY_LAYERS;
			range.levelCount = VK_REMAINING_MIP_LEVELS;

			VkImageMemoryBarrier2 barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(source_dependency_state.stage);
			barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(target_dependency_state.stage);
			barrier.srcAccessMask = VulkanUnitConverter::DependencyToVkAccess(source_dependency_state.access_type);
			barrier.dstAccessMask = VulkanUnitConverter::DependencyToVkAccess(target_dependency_state.access_type);
			barrier.image = vk_image;
			barrier.subresourceRange = range;
			barrier.oldLayout = VulkanUnitConverter::RenderStateToTextureLayout(source_dependency_state.desired_state);
			barrier.newLayout = VulkanUnitConverter::RenderStateToTextureLayout(target_dependency_state.desired_state);

			if (source_dependency_state.desired_state != target_dependency_state.desired_state) {
				barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
			}

			image_barriers.push_back(barrier);
		}

		break;
	}
	default:
		throw std::runtime_error("Invalid resource type.\n");
	}
}

void VulkanBarrier::AddGlobalMemoryBarrier(const VulkanDependencyState& source_dependency_state, const VulkanDependencyState& target_dependency_state) {
	VkMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	barrier.dstAccessMask = VulkanUnitConverter::DependencyToVkAccess(target_dependency_state.access_type);
	barrier.srcAccessMask = VulkanUnitConverter::DependencyToVkAccess(source_dependency_state.access_type); // The store has a single canonical access type
	barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(source_dependency_state.stage);
	barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(target_dependency_state.stage);

	global_memory_barriers.push_back(barrier);
}

void VulkanDrawState::SetMaterialResources(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t bind_id)
{

	auto& params = list->GetCurrentPipeline()->GetSignature().GetDescriptor().parameters; 
	if(params[bind_id].material_template != material->GetMaterialTemplate()) {
		throw std::runtime_error("Invalid material bound to id " + std::to_string(bind_id) + ".\n");
	}

	auto material_template = material->GetMaterialTemplate();
	auto vk_mat = std::static_pointer_cast<VulkanMaterial>(material);

	VulkanDependencyState dep = {};
	dep.access_type = VulkanCommandListDependencyType::READ;
	dep.desired_state = RenderState::COMMON;

	used_descriptor_tables.insert(vk_mat->GetDescriptorTable());

	auto fnd = draw_resources.find(bind_id);
	unsigned int dependency_index = 0;
	bool new_binding = false;
	if(fnd == draw_resources.end()) {
		draw_resources.insert_or_assign(bind_id, pending_dependencies.size());
		currently_bound_count++;
		new_binding = true;
	} else {
		dependency_index = fnd->second;
	}

	for (int i = 0; i < material->GetMaterialParameters().size(); i++) {
		std::shared_ptr<RenderResource> resource;
		auto& parameter = material->GetMaterialParameters()[i];
		switch (parameter.type)
		{
		case MaterialLayoutItemType::CONSTANT_BUFFER:
			resource = std::get<std::shared_ptr<RenderBufferResource>>(parameter.resource);
			break;
		case MaterialLayoutItemType::STORAGE_BUFFER:
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
		dep.access_type = parameter.type == MaterialLayoutItemType::STORAGE_BUFFER ? VulkanCommandListDependencyType::WRITE | VulkanCommandListDependencyType::READ : VulkanCommandListDependencyType::READ;

		VulkanDrawResource res;
		res.resource = resource;
		res.dependency_target_state = dep;

		AssertIndividualResourceValidity(resource);
		if(new_binding) {
			pending_dependencies.push_back(res);
		} else {
			pending_dependencies[dependency_index++] = res;
		}
	}

	if(vk_mat->GetConstantBuffer()) {
		dep.desired_state = RenderState::COMMON;

		VulkanDrawResource res;
		res.resource = vk_mat->GetConstantBuffer();
		res.dependency_target_state = dep;
		AssertIndividualResourceValidity(res.resource);
		if(new_binding) {
			pending_dependencies.push_back(res);
		} else {
			pending_dependencies[dependency_index++] = res;
		}
	}

	dirty = true;
}


bool VulkanDrawState::IsPipelineReady()
{
    return currently_bound_count == expected_binding_count;
}

void VulkanDrawState::AssertIndividualResourceValidity(std::shared_ptr<RenderResource> resource) {
	for(auto& store : pending_stores) {
		if(store == resource->GetResourceStore()) {
			throw std::runtime_error("Individual resources belonging to a resource store and the store itself cannot be used in the same pipeline.\n");
		}
	}
}

//TODO: Separate draw and dispatch dependencies.
void VulkanDependencyHandler::FlushDrawDependencies(VulkanRenderCommandList* list)
{
	if(!draw_state.IsPipelineReady()) {
		throw std::runtime_error("Pipeline Resources were not fully bound before pipeline usage.\n");
	}

	if(list->IsRenderPassActive() && !draw_state.dirty) {
		return;
	}

	auto pipeline_bind_point = std::static_pointer_cast<VulkanPipeline>(list->GetCurrentPipeline()->GetPipelineNativeExtension())->GetBindPoint();

	bool forced_barrier_emission = false;

	barrier.ClearBarriers();

	if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
		PrepareDependencyForEmission(list, list->GetVertexBuffer(),  {VulkanCommandListDependencyType::READ, RenderState::IN_USE_VERTEX_BUFFER});
		PrepareDependencyForEmission(list, list->GetIndexBuffer(),  {VulkanCommandListDependencyType::READ, RenderState::IN_USE_INDEX_BUFFER});

		if (!list->GetVertexBuffer()) {
			throw std::runtime_error("Vertex buffer was not set on the pipeline.\n");
		}

		if (!list->GetVertexBuffer()) {
			throw std::runtime_error("Index buffer was not set on the pipeline.\n");
		}

		if(framebuffer_dependency_pending) {
			auto framebuffer_desc = list->GetCurrentFrameBuffer()->GetBufferDescriptor();

			if(framebuffer_desc.depth_stencil_attachment.resource) {
				PrepareDependencyForEmission(list, framebuffer_desc.depth_stencil_attachment.resource, {VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_DEPTH_STENCIL_ATTACHMENT});
			}

			for(auto color_attachment : framebuffer_desc.color_attachments) {
				PrepareDependencyForEmission(list, color_attachment.resource, {VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_COLOR_ATTACHMENT});
			}

			framebuffer_dependency_pending = false;
		}
	}

	for (auto& dep : draw_state.pending_dependencies) {
		PrepareDependencyForEmission(list, dep.resource, dep.dependency_target_state);
	}

	for(auto& store : draw_state.pending_stores) {
		PrepareStoreDependencyForEmission(list, store);
	}

	FlushPreparedDependencies(list);

	draw_state.dirty = false;
}

void VulkanDependencyHandler::PipelineChange(VulkanRenderCommandList *list, std::shared_ptr<Pipeline> new_pipeline)
{
	auto current_pipeline = list->GetCurrentPipeline();
	// If pipeline signatures dont match invalidate the current dependencies, we compare pointers, since signatures should originate from the same layout
	if (!current_pipeline || &current_pipeline->GetSignature() != &new_pipeline->GetSignature()) {
		draw_state.InvalidateDrawDependencies(list, new_pipeline);
	}
}

void VulkanDependencyHandler::RenderTargetChange(VulkanRenderCommandList *list, std::shared_ptr<RenderFrameBufferResource> new_framebuffer)
{
	framebuffer_dependency_pending = true;
}

void VulkanDrawState::InvalidateDrawDependencies(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline)
{
	draw_resources.clear();
	pending_dependencies.clear();
	pending_stores.clear();
	currently_bound_count = 0;
	expected_binding_count = new_pipeline->GetSignature().GetDescriptor().parameters.size();
	dirty = true;
}

bool VulkanDependencyHandler::IsPipelineReady()
{
	return draw_state.IsPipelineReady();
}

void VulkanDependencyHandler::EnsureInitialization(std::shared_ptr<RenderResource> resource) {
	auto fnd = individual_resource_dependencies_map.find(resource);
	if(fnd == individual_resource_dependencies_map.end()) {
		auto dependency = GetNewIndividualResourceRecord(resource);
		auto& dependency_state = individual_resource_dependency_storage[dependency->second].state;
		dependency_state.allow_uninitialized = false;
	} else {
		individual_resource_dependency_storage[fnd->second].state.allow_uninitialized = false;
	}
}

VulkanCommandListDependencyState VulkanDependencyHandler::GetDependency(std::shared_ptr<RenderResource> resource)
{
	auto fnd = individual_resource_dependencies_map.find(resource);
	if (fnd != individual_resource_dependencies_map.end()) {
		return individual_resource_dependency_storage[fnd->second].state;
	}

	return VulkanCommandListDependencyState();
}

VulkanDependencyHandler::VulkanDependencyHandlerFeedback VulkanDependencyHandler::FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value)
{
	DEFINE_VK_INSTANCE(context);
	uint64_t timeline_requirement = 0;
	VulkanRenderResource* resource;
	VulkanRenderCommandList* vk_command_list = static_cast<VulkanRenderCommandList*>(list);
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	for (auto& dependency_entry : individual_resource_dependencies_map) {
		auto& dependency = individual_resource_dependency_storage[dependency_entry.second].state;
		resource = static_cast<VulkanRenderResource*>(dependency_entry.first->GetExtensionData());

		if (!dependency.allow_uninitialized && dependency_entry.first->GetRenderState() == RenderState::UNINITIALIZED) { // If we allow uninitialed resource then accept the resource
			throw std::runtime_error("Attempted to read an uninitialized resource or the resource changed type between command recording and command list submit.\n");
		}

		auto old_default_state = resource->GetDefaultState();
		auto requested_default_state = dependency.desired_final_state;
		auto new_state = requested_default_state == RenderState::EMPTY ? old_default_state : requested_default_state;
		
		if(old_default_state != dependency.expected_state) {
			throw std::runtime_error("Default state of a resource was changed before command buffer was submitted which invalidated the command buffer, check for misplaced usage of SetDefaultResouce.\n");
		}
		
		dependency_entry.first->SetRenderState(new_state); // Change the resource back to its default state, this also serves to mark the resource initialized
		
		bool transitioned = false;

		if (dependency_entry.first->GetExtensionData()->IsTexture()) {
			auto texture = static_cast<VulkanRenderTextureResource*>(dependency_entry.first->GetExtensionData());
			if(requested_default_state != RenderState::EMPTY) {
				texture->default_state = requested_default_state;
			}
			if (new_state != dependency.current_state) {
				VkImageSubresourceRange range;
				range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(texture->GetFormat()) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseArrayLayer = 0;
				range.baseMipLevel = 0;
				range.layerCount = VK_REMAINING_ARRAY_LAYERS;
				range.levelCount = VK_REMAINING_MIP_LEVELS; 
				
				manager->TransitionImage(list, texture, range, dependency.current_state, new_state, PipelineStage::ALL_STAGES, PipelineStage::ALL_STAGES);
				transitioned = true;
			}
		}

		if(transitioned) {
			dependency.type |= VulkanCommandListDependencyType::WRITE; // Transition counts as write
		}

		uint64_t read_timeline = resource->read_timeline;
		uint64_t write_timeline = resource->write_timeline;

		if((dependency.type & VulkanCommandListDependencyType::READ) != VulkanCommandListDependencyType::NONE) {
			timeline_requirement = std::max(write_timeline, timeline_requirement); // On read we need to wait for all writes to finish, we dont care about other reads
			resource->read_timeline = new_timeline_value;
		}

		if((dependency.type & VulkanCommandListDependencyType::WRITE) != VulkanCommandListDependencyType::NONE) {
			timeline_requirement = std::max(std::max(write_timeline, read_timeline), timeline_requirement); // On write we need to wait for reads as well
			resource->write_timeline = new_timeline_value;
		}

		if((dependency.type & VulkanCommandListDependencyType::INVALID) != VulkanCommandListDependencyType::NONE) {
			throw std::runtime_error("Invalid dependency type.\n");
		}
	}

	for(auto store : resource_store_dependencies_map) {
		auto vk_store = std::static_pointer_cast<VulkanRenderResourceStore>(store.first);
		timeline_requirement = std::max(timeline_requirement, vk_store->GetTimelineValue());
		vk_store->SetTimelineValue(new_timeline_value);
	}

	for(auto table : draw_state.used_descriptor_tables) {
		auto vk_desc_table = table;
		vk_desc_table->timeline = new_timeline_value;
	}

	VulkanDependencyHandlerFeedback feedback;
	feedback.timeline_wait = timeline_requirement;

	return feedback;
}

void VulkanRenderCommandList::ResetState()
{
	dependency_handler.Reset();
	are_index_vertex_buffers_bound = false;
	current_framebuffer.reset();
	current_pipeline.reset();
	index_buffer.reset();
	vertex_buffer.reset();
	is_scissorrect_defined = false;
	is_viewport_defined = false;
	render_pass_active = false;
	render_pass_counter = 0;
	scissor_rect = RenderScissorRect({0,0}, {0,0});
	viewport = RenderViewport({0,0}, {0,0}, 0.0f, 0.0f);
	timeline_submitted = 0;
	submission_callbacks.clear();
}

void VulkanRenderCommandList::ResetCommandBuffer()
{
	vkResetCommandBuffer(command_buffer, NULL);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(command_buffer, &begin_info);
}

void VulkanDependencyHandler::Reset()
{
	individual_resource_dependency_storage.clear();
	individual_resource_dependencies_map.clear();
	resource_store_dependencies_map.clear();
	draw_state = VulkanDrawState();
	framebuffer_dependency_pending = true;
}

void VulkanDependencyHandler::AddIndividualResourceOverride(int individual_resource_index) {
	auto& dependency = individual_resource_dependency_storage[individual_resource_index];
	if(dependency.store_it.has_value()) {
		auto& store = dependency.store_it.value()->second;
		if(dependency.store_version >= store.store_version) {
			return;
		}

		dependency.next_resource = store.first_resource_override;
		store.first_resource_override = individual_resource_index;
		dependency.store_version = store.store_version;
	}
}
