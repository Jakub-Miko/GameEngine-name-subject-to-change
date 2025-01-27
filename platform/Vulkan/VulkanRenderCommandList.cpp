#include "VulkanRenderCommandList.h"
#include "VulkanRenderCommandAllocator.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderContext.h"
#include "VulkanRenderResourceManager.h"

VulkanRenderCommandList::VulkanRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc) : RenderCommandList(renderer, alloc), dependency_handler(), current_framebuffer(nullptr)
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
}

void VulkanRenderCommandList::Execute()
{
}

void VulkanRenderCommandList::SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer)
{
}

void VulkanRenderCommandList::SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer)
{
}

void VulkanRenderCommandList::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture)
{
}

void VulkanRenderCommandList::SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture)
{
}

void VulkanRenderCommandList::SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture)
{
}

void VulkanRenderCommandList::SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer)
{
	current_framebuffer = framebuffer;
}

void VulkanRenderCommandList::SetDefaultRenderTarget()
{
	DEFINE_VK_INSTANCE(context);
	current_framebuffer = context->default_framebuffers[context->current_framebuffer];
}

void VulkanRenderCommandList::Clear()
{
	VulkanCommandListDependency dep(VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST, RenderState::UNINITIALIZED);
	auto& desc = current_framebuffer->GetBufferDescriptor();
	VkClearColorValue clear{ {0,0,0,0} };
	VkClearDepthStencilValue clear_depth { 1.0,0 };
	if (current_framebuffer) {
		for (auto color_attachment : desc.color_attachments) {
			auto vk_res = static_cast<VulkanRenderTextureResource*>(color_attachment.resource->GetExtensionData());
			dependency_handler->AddDependency(this, color_attachment.resource, dep);
			VkImageSubresourceRange range;
			range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.layerCount = VK_REMAINING_ARRAY_LAYERS;
			range.baseMipLevel = color_attachment.level;
			range.levelCount = 1;

			vkCmdClearColorImage(command_buffer, vk_res->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

		}

		auto vk_depth_res = static_cast<VulkanRenderTextureResource*>(desc.depth_stencil_attachment.resource->GetExtensionData());
		dependency_handler->AddDependency(this, desc.depth_stencil_attachment.resource, dep);
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
}

void VulkanRenderCommandList::SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer)
{
}

void VulkanRenderCommandList::SetScissorRect(const RenderScissorRect& scissor_rect)
{
}

void VulkanRenderCommandList::SetViewport(const RenderViewport& viewport)
{
}

void VulkanRenderCommandList::SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table)
{
}

void VulkanRenderCommandList::GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture)
{
}

void VulkanRenderCommandList::Draw(uint32_t index_count, bool use_unsined_short_as_index, int index_offset)
{
}

void VulkanRenderCommandList::DrawArray(uint32_t vertex_count)
{
}

void VulkanRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
{
}

void VulkanRenderCommandList::DrawSquare(const glm::mat4& transform, glm::vec4 color)
{
}

VulkanCommandListDependency VulkanRenderCommandList::AddDependency(std::shared_ptr<RenderResource> dep_resource, VulkanCommandListDependency dep)
{
	return dependency_handler->AddDependency(this,dep_resource, dep);
}	

VulkanCommandListDependency VulkanRenderCommandList::GetDependency(std::shared_ptr<RenderResource> dep_resource)
{
	return dependency_handler->GetDependency(dep_resource);
}

VulkanCommandListDependency DefaultVulkanDependencyHandler::AddDependency(RenderCommandList* list, std::shared_ptr<RenderResource> resource, 
	VulkanCommandListDependency dependency,VulkanCommandListDependencyExtra extra)
{
	auto fnd = dependencies.find(resource);
	VulkanCommandListDependency current_dep; 
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());

	if (fnd != dependencies.end()) { // never overwrite the expected value, only the first command matters
		current_dep = fnd->second;
		fnd->second.previous_access = dependency.type;
		if (dependency.type == VulkanCommandListDependencyType::WRITE) { // Make sure Read doesnt overwrite write
			fnd->second.type = dependency.type;
		}
		fnd->second.current_state = dependency.current_state;
	} else {
		dependency.previous_access = dependency.type;
		dependencies.insert(std::make_pair(resource, dependency));
		current_dep = VulkanCommandListDependency(VulkanCommandListDependencyType::INVALID, RenderState::EMPTY, RenderState::EMPTY);
	}

	switch (resource->GetResourceType())
	{
	case RenderResourceType::RenderBufferResource:
	{
		if (current_dep.type == VulkanCommandListDependencyType::INVALID) {
			break; // First access to a buffer resource is implicityly synchronized and all memory is always visible so we dont need to do anything
		}
		
		VulkanRenderBufferResource* vk_resource = static_cast<VulkanRenderBufferResource*>(resource.get());
		if (current_dep.previous_access == VulkanCommandListDependencyType::WRITE && dependency.type == VulkanCommandListDependencyType::READ) { //Synchronize and Make Data available
			manager->BufferBarrier(list, std::static_pointer_cast<RenderBufferResource>(resource), true, extra.source_stage, extra.target_stage);
		}
		else if (current_dep.previous_access != VulkanCommandListDependencyType::READ || dependency.type != VulkanCommandListDependencyType::READ) { // for write after write, or write affter read, no visibility operations are required, but we must ensure ordering
			manager->BufferBarrier(list, std::static_pointer_cast<RenderBufferResource>(resource), false, extra.source_stage, extra.target_stage);
		}
		break;
	}
	case RenderResourceType::RenderTexture2DResource:
	case RenderResourceType::RenderTexture2DArrayResource:
	case RenderResourceType::RenderTexture2DCubemapResource:
	{
		// In the first access, execution and memory_barrier are always false, but we may need to transition the image.
		VulkanRenderTextureResource* vk_resource = static_cast<VulkanRenderTextureResource*>(resource->GetExtensionData());
		bool transition = current_dep.current_state != dependency.current_state; // if the requested type differs then change it.
		bool execution_barrier = current_dep.previous_access != VulkanCommandListDependencyType::READ || dependency.type != VulkanCommandListDependencyType::READ; // if the requested type differs then change it.
		bool memory_barrier = current_dep.previous_access == VulkanCommandListDependencyType::WRITE && dependency.type == VulkanCommandListDependencyType::READ; // if we need to read after write then use a memory barrier

		RenderState source = current_dep.type == VulkanCommandListDependencyType::INVALID ? resource->GetRenderState() : current_dep.current_state;
		memory_barrier |= transition; //transition is a write

		VkImageSubresourceRange range;
		range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(vk_resource->GetFormat()) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = VK_REMAINING_ARRAY_LAYERS;
		range.levelCount = VK_REMAINING_MIP_LEVELS;

		if (transition || memory_barrier || execution_barrier) {
			manager->TransitionImage(list, vk_resource, range, source, dependency.current_state, memory_barrier,
				extra.source_stage, extra.target_stage);
		}

		break;
	}
	default:
		throw std::runtime_error("Invalid resource type.\n");
	}




	return current_dep;
}

VulkanCommandListDependency DefaultVulkanDependencyHandler::GetDependency(std::shared_ptr<RenderResource> resource)
{
	auto fnd = dependencies.find(resource);
	if (fnd != dependencies.end()) {
		return fnd->second;
	}

	return VulkanCommandListDependency(VulkanCommandListDependencyType::INVALID, RenderState::EMPTY, RenderState::EMPTY);
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

		if (dependency.second.expected_state != RenderState::UNINITIALIZED) { // If we allow uninitialed resource then accept the resource
			if (dependency.second.expected_state != dependency.first->GetRenderState()) { // If we don't the resource must be in the default state
				throw std::runtime_error("Attempted to read an uninitialized resource or the resource change type between command recording and command list submit.\n");
			}
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
		default:
			throw std::runtime_error("Invalid dependency type.\n");
		}

		dependency.first->SetRenderState(resource->GetDefaultState()); // Change the resource back to its default state, this also serves to mark the resource initialized

		if (dependency.first->GetExtensionData()->IsTexture()) {
			auto texture = static_cast<VulkanRenderTextureResource*>(dependency.first->GetExtensionData());
			if (texture->default_state != dependency.second.current_state) {
				VkImageSubresourceRange range;
				range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(texture->GetFormat()) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseArrayLayer = 0;
				range.baseMipLevel = 0;
				range.layerCount = VK_REMAINING_ARRAY_LAYERS;
				range.levelCount = VK_REMAINING_MIP_LEVELS;
				
				manager->TransitionImage(list, texture, range, dependency.second.current_state, texture->default_state, false, PipelineStage::PIPELINE_TOP, PipelineStage::PIPELINE_BOTTOM);
			}
		}
	}

	VulkanDependencyHandlerFeedback feedback;
	feedback.timeline_wait = timeline_requirement;

	return feedback;
}

void DefaultVulkanDependencyHandler::Reset()
{
	dependencies.clear();
	non_dependent_resources.clear();
}
