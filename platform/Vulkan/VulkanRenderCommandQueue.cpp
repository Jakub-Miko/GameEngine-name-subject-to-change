#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderContext.h"
#include "VulkanRenderFence.h"
#include "VulkanRenderResource.h"

void VulkanRenderCommandQueue::ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists)
{
	std::vector<VkCommandBuffer> buffers;
	buffers.reserve(lists.size());
	for (int i = 0; i < lists.size(); i++) {
		buffers.push_back(*static_cast<VulkanRenderCommandList*>(lists[i])->GetVkCommandBuffer());
		vkEndCommandBuffer(*static_cast<VulkanRenderCommandList*>(lists[i])->GetVkCommandBuffer());
	}

	DEFINE_VK_INSTANCE(context);


	uint64_t value = ++last_buffer_signaled;
	VkTimelineSemaphoreSubmitInfo submit_sync = {};
	submit_sync.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	submit_sync.signalSemaphoreValueCount = 1;
	submit_sync.pSignalSemaphoreValues = &value; //should i care ?  https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#limits-maxTimelineSemaphoreValueDifference

	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = &submit_sync;
	info.commandBufferCount = buffers.size();
	info.pCommandBuffers = buffers.data();
	info.pSignalSemaphores = static_cast<VulkanRenderFence*>(command_buffer_fence.get())->GetSemaphore();;
	info.signalSemaphoreCount = 1;
	info.pWaitSemaphores = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitDstStageMask = NULL;

	submit_mutex.lock();

	for (auto list : lists) {
		VulkanRenderResource* resource;
		VulkanRenderCommandList* vk_command_list = static_cast<VulkanRenderCommandList*>(list);
		for (auto& dependency : vk_command_list->command_list_dependencies) {
			resource = static_cast<VulkanRenderResource*>(dependency.resource->GetExtensionData());
			switch (dependency.type)
			{
			case VulkanRenderCommandList::VulkanCommandListDependencyType::READ:
				resource->read_timeline = value;
				break;
			case VulkanRenderCommandList::VulkanCommandListDependencyType::WRITE:
				resource->read_timeline = value;
				resource->write_timeline = value;
				break;
			default:
				throw std::runtime_error("Invalid dependency type.\n");
			}
		}
	}


	vkQueueSubmit(vk_queue, 1, &info, NULL);
	submit_mutex.unlock();
}

void VulkanRenderCommandQueue::ExecuteRenderCommandList(RenderCommandList* list)
{
	DEFINE_VK_INSTANCE(context);
	VulkanRenderCommandList* vk_command_list = static_cast<VulkanRenderCommandList*>(list);
	vkEndCommandBuffer(*vk_command_list->GetVkCommandBuffer());
	uint64_t value = ++last_buffer_signaled;
	VkTimelineSemaphoreSubmitInfo submit_sync = {};
	submit_sync.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	submit_sync.signalSemaphoreValueCount = 1;
	submit_sync.pSignalSemaphoreValues = &value;

	
	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = &submit_sync;
	info.commandBufferCount = 1;
	info.pCommandBuffers = vk_command_list->GetVkCommandBuffer();
	info.pSignalSemaphores = static_cast<VulkanRenderFence*>(command_buffer_fence.get())->GetSemaphore();;
	info.signalSemaphoreCount = 1;
	info.pWaitSemaphores = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitDstStageMask = NULL;


	submit_mutex.lock();
	
	VulkanRenderResource* resource;
	for (auto& dependency : vk_command_list->command_list_dependencies) {
		resource = static_cast<VulkanRenderResource*>(dependency.resource->GetExtensionData());
		switch (dependency.type)
		{
		case VulkanRenderCommandList::VulkanCommandListDependencyType::READ:
			resource->read_timeline = value;
			break;
		case VulkanRenderCommandList::VulkanCommandListDependencyType::WRITE:
			resource->read_timeline = value;
			resource->write_timeline = value;
			break;
		default:
			throw std::runtime_error("Invalid dependency type.\n");
		}
	}

	vkQueueSubmit(vk_queue, 1, &info, NULL);
	submit_mutex.unlock();
}

void VulkanRenderCommandQueue::Signal(std::shared_ptr<RenderFence> fence, int num)
{
	uint64_t value = num;

	VkTimelineSemaphoreSubmitInfo type_info;
	type_info.pNext = NULL;
	type_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	type_info.pWaitSemaphoreValues = NULL;
	type_info.waitSemaphoreValueCount = 0;
	type_info.signalSemaphoreValueCount = 1;
	type_info.pSignalSemaphoreValues = &value;

	
	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = &type_info;
	info.commandBufferCount = 0;
	info.pCommandBuffers = NULL;
	info.pSignalSemaphores = static_cast<VulkanRenderFence*>(fence.get())->GetVkSemaphore();
	info.signalSemaphoreCount = 1;
	info.pWaitSemaphores = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitDstStageMask = NULL;

	vkQueueSubmit(vk_queue, 1, &info, NULL);
}

void VulkanRenderCommandQueue::VkBinarySemaphoreSignal(VkSemaphore semaphore)
{

	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = NULL;
	info.commandBufferCount = 0;
	info.pCommandBuffers = NULL;
	info.pSignalSemaphores = &semaphore;
	info.signalSemaphoreCount = 1;
	info.pWaitSemaphores = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitDstStageMask = NULL;

	vkQueueSubmit(vk_queue, 1, &info, NULL);
}

void VulkanRenderCommandQueue::VkBinarySemaphoreWait(VkSemaphore semaphore, VkPipelineStageFlags wait_mask)
{
	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = NULL;
	info.commandBufferCount = 0;
	info.pCommandBuffers = NULL;
	info.pSignalSemaphores = NULL;
	info.signalSemaphoreCount = 0;
	info.pWaitSemaphores = &semaphore;
	info.waitSemaphoreCount = 1;
	info.pWaitDstStageMask = &wait_mask;

	vkQueueSubmit(vk_queue, 1, &info, NULL);
}

VulkanRenderCommandQueue::VulkanRenderCommandQueue(VkQueue queue) : vk_queue(queue), submit_mutex(), command_buffer_fence()
{
	command_buffer_fence.reset(RenderFence::CreateFence());
}

void VulkanRenderCommandQueue::Present()
{
	DEFINE_VK_INSTANCE(context);
	context->SignalEndFrame();
	auto semaphore = context->GetVkRenderSemaphore();

	uint32_t index = context->GetCurrentFramebufferIndex();

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	presentInfo.pSwapchains = context->GetVkSwapchain();
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &index;

	vkQueuePresentKHR(vk_queue, &presentInfo);

	context->StartNewFrame();

}

