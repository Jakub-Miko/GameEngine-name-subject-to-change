#include "VulkanRenderCommandAllocator.h"
#include "VulkanRenderCommandAllocator.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderContext.h"
#include "VkBootstrap.h"

void* VulkanRenderCommandAllocator::Get()
{
	return &pool;
}

void VulkanRenderCommandAllocator::clear()
{
	DEFINE_VK_INSTANCE(context);
	vkResetCommandPool(context->GetVkDevice(), pool, NULL);
}

VulkanRenderCommandAllocator::VulkanRenderCommandAllocator(size_t starting_size)
{
	DEFINE_VK_INSTANCE(context);

	VkCommandPoolCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = NULL;
	info.queueFamilyIndex = context->GetVkbDevice().get_queue_index(vkb::QueueType::graphics).value();
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(context->GetVkDevice(), &info, NULL, &pool);
}

VulkanRenderCommandAllocator::~VulkanRenderCommandAllocator()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroyCommandPool(context->GetVkDevice(), pool, NULL);
}
