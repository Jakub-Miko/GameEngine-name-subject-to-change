#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderContext.h"
#include "VulkanRenderFence.h"

void VulkanRenderCommandQueue::ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists)
{
	std::vector<VkCommandBuffer> buffers;
	buffers.reserve(lists.size());
	for (int i = 0; i < lists.size(); i++) {
		buffers.push_back(*static_cast<VulkanRenderCommandList*>(lists[i])->GetVkCommandBuffer());
	}

	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = NULL;
	info.commandBufferCount = lists.size();
	info.pCommandBuffers = buffers.data();
	info.pSignalSemaphores = NULL;
	info.signalSemaphoreCount = 0;
	info.pWaitSemaphores = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitDstStageMask = NULL;

	vkQueueSubmit(vk_queue, 1, &info, NULL);
}

void VulkanRenderCommandQueue::ExecuteRenderCommandList(RenderCommandList* list)
{
	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = NULL;
	info.commandBufferCount = 1;
	info.pCommandBuffers = static_cast<VulkanRenderCommandList*>(list)->GetVkCommandBuffer();
	info.pSignalSemaphores = NULL;
	info.signalSemaphoreCount = 0;
	info.pWaitSemaphores = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitDstStageMask = NULL;

	vkQueueSubmit(vk_queue, 1, &info, NULL);
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

