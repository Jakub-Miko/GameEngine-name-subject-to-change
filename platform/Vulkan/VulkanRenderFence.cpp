#include "VulkanRenderFence.h"
#include <Profiler.h>
#include "VulkanRenderContext.h"
#include <stdexcept>

bool VulkanRenderFence::WaitForValue(int desired_value)
{
	DEFINE_VK_INSTANCE(context);
	uint64_t value = desired_value;

	VkSemaphoreWaitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	info.pNext = NULL;
	info.semaphoreCount = 1;
	info.pSemaphores = &semaphore;
	info.pValues = &value;
	
	VkResult code = vkWaitSemaphores(context->GetVkDevice(), &info, 30000000000); // timeout 30 seconds
	if (code != VK_SUCCESS) {
		throw std::runtime_error("Fence wait timed out.\n");
	}
	return true;
}

void VulkanRenderFence::Wait()
{
	WaitForValue(GetValue() + 1);
}

int VulkanRenderFence::GetValue()
{
	DEFINE_VK_INSTANCE(context);
	uint64_t value = 0;
	vkGetSemaphoreCounterValue(context->GetVkDevice(), semaphore, &value);
	return value;
}

VulkanRenderFence::VulkanRenderFence(uint32_t initial_value) : semaphore()
{
	DEFINE_VK_INSTANCE(context);

	VkSemaphoreTypeCreateInfo type_info = {};
	type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	type_info.initialValue = initial_value;
	type_info.pNext = nullptr;
	type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = &type_info;
	info.flags = 0;

	vkCreateSemaphore(context->GetVkDevice(), &info, nullptr, &semaphore);

}

VulkanRenderFence::~VulkanRenderFence()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroySemaphore(context->GetVkDevice(), semaphore, nullptr);
}

void VulkanRenderFence::Signal(int num)
{
	DEFINE_VK_INSTANCE(context);

	VkSemaphoreSignalInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	info.pNext = nullptr;
	info.value = num;
	info.semaphore = semaphore;


	vkSignalSemaphore(context->GetVkDevice(), &info);
}

