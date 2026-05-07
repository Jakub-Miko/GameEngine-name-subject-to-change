 #include "VulkanRenderCommandQueue.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderContext.h"
#include "VulkanRenderFence.h"
#include "VulkanRenderResource.h"
#include "VulkanRenderResourceManager.h"

void VulkanRenderCommandQueue::ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists)
{
	throw std::runtime_error("Not implemented.\n");
}

void VulkanRenderCommandQueue::ExecuteRenderCommandList(std::shared_ptr<RenderCommandList> list)
{
	DEFINE_VK_INSTANCE(context);
	auto vk_command_list = std::static_pointer_cast<VulkanRenderCommandList>(list);
	submit_mutex.lock();
	uint64_t value = ++last_buffer_signaled; /// @todo this can cause a datarace make sure its locked behind the submission mutex
	VkTimelineSemaphoreSubmitInfo submit_sync = {};

	vk_command_list->OutsideRenderPass();

	submit_sync.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	submit_sync.signalSemaphoreValueCount = 1;
	submit_sync.pSignalSemaphoreValues = &value;

	
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = &submit_sync;
	info.commandBufferCount = 1;
	info.pCommandBuffers = vk_command_list->GetVkCommandBuffer();
	info.pSignalSemaphores = static_cast<VulkanRenderFence*>(command_buffer_fence.get())->GetSemaphore();;
	info.signalSemaphoreCount = 1;
	info.pWaitDstStageMask = NULL;
	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = NULL;

	auto sync = vk_command_list->dependency_handler.FinalizeDependencies(vk_command_list.get(), value);

	VkPipelineStageFlags wait_flags = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	if (sync.timeline_wait > context->GetCurrentGpuTimelineValue()) { // we need to wait until the timeline requirement is met before executing this command list
		info.waitSemaphoreCount = 1;
		info.pWaitDstStageMask = &wait_flags;
		info.pWaitSemaphores = static_cast<VulkanRenderFence*>(command_buffer_fence.get())->GetSemaphore();
		submit_sync.waitSemaphoreValueCount = 1;
		submit_sync.pWaitSemaphoreValues = &sync.timeline_wait;
	}

	vk_command_list->OutsideRenderPass(); // Make sure to end the render pass before submission;
	vkEndCommandBuffer(*vk_command_list->GetVkCommandBuffer());

	for(auto& callback : vk_command_list->submission_callbacks) {
		callback();
	}
	vk_command_list->submission_callbacks.clear();
	vkQueueSubmit(vk_queue, 1, &info, NULL);
	vk_command_list->ResetState(); // Reset all handles held by the abstraction since the abstraction no longer manages them after submit, but retain the actual command buffer.
	vk_command_list->timeline_submitted = value; // Make sure to update the timeline value after state reset otherwise it will be reset to 0 and the the Render API will think it is not pending(Which can apparently cause AMD drivers to crash)


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

	submit_mutex.lock();
	vkQueueSubmit(vk_queue, 1, &info, NULL);
	submit_mutex.unlock();
}

void VulkanRenderCommandQueue::WaitForValue(uint32_t value)
{
	command_buffer_fence->WaitForValue(value);
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

	submit_mutex.lock();
	vkQueueSubmit(vk_queue, 1, &info, NULL);
	submit_mutex.unlock();
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

	submit_mutex.lock();
	vkQueueSubmit(vk_queue, 1, &info, NULL);
	submit_mutex.unlock();
}

VulkanRenderCommandQueue::VulkanRenderCommandQueue(VkQueue queue) : vk_queue(queue), submit_mutex(), command_buffer_fence()
{
	command_buffer_fence.reset(RenderFence::CreateFence(1)); // Create a queue with initial value of one to ensure that 0 is always completed.
}

