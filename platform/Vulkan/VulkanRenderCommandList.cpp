#include "VulkanRenderCommandList.h"
#include "VulkanRenderCommandAllocator.h"
#include "VulkanRenderContext.h"

VulkanRenderCommandList::VulkanRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc) : RenderCommandList(renderer, alloc), command_list_dependencies()
{
	DEFINE_VK_INSTANCE(context);

	VkCommandBufferAllocateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = NULL;
	info.commandPool = *(static_cast<VkCommandPool*>(alloc->Get()));
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;

	vkAllocateCommandBuffers(context->GetVkDevice(), &info, &command_buffer);
	
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	SetDefaultRenderTarget();

	vkBeginCommandBuffer(command_buffer, &begin_info);
}

VulkanRenderCommandList::~VulkanRenderCommandList()
{
	DEFINE_VK_INSTANCE(context);

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

VulkanRenderCommandList::VulkanCommandListDependency VulkanRenderCommandList::AddDependency(std::shared_ptr<RenderResource> dep_resource, VulkanCommandListDependency dep)
{
	auto fnd = command_list_dependencies.find(dep_resource);
	if (fnd != command_list_dependencies.end()) { // never overwrite the expected value, only the first command matters
		VulkanRenderCommandList::VulkanCommandListDependency temp = fnd->second;
		fnd->second.previous_access = dep.type;
		if (dep.type == VulkanRenderCommandList::VulkanCommandListDependencyType::WRITE) { // Make sure Read doesnt overwrite write
			fnd->second.type = dep.type;
		}
		fnd->second.current_state = dep.current_state;
		return temp;
	}
	
	dep.previous_access = dep.type;

	command_list_dependencies.insert(std::make_pair(dep_resource, dep));

	return VulkanRenderCommandList::VulkanCommandListDependency{ VulkanRenderCommandList::VulkanCommandListDependencyType::INVALID };
}	

VulkanRenderCommandList::VulkanCommandListDependency VulkanRenderCommandList::GetDependency(std::shared_ptr<RenderResource> dep_resource)
{
	auto fnd = command_list_dependencies.find(dep_resource);
	if (fnd != command_list_dependencies.end()) {
		return fnd->second;
	}

	return VulkanRenderCommandList::VulkanCommandListDependency{ VulkanRenderCommandList::VulkanCommandListDependencyType::INVALID };
}
