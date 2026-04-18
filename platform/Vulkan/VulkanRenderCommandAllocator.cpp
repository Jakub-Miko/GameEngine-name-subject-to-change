#include "VulkanRenderCommandAllocator.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderContext.h"
#include "VulkanRenderResourceManager.h"
#include "VkBootstrap.h"

void VulkanRenderCommandAllocator::clear()
{
	DEFINE_VK_INSTANCE(context);
	vkResetCommandPool(context->GetVkDevice(), pool, 0);
}

VulkanRenderCommandAllocator::VulkanRenderCommandAllocator(size_t starting_size) : free_command_lists(), allocation_mutex()
{
	DEFINE_VK_INSTANCE(context);

	VkCommandPoolCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = NULL;
	info.queueFamilyIndex = context->GetVkbDevice().get_queue_index(vkb::QueueType::graphics).value();
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(context->GetVkDevice(), &info, nullptr, &pool);
}

std::shared_ptr<RenderCommandList> VulkanRenderCommandAllocator::GetCommandList()
{
	std::lock_guard<std::mutex> lock(allocation_mutex);
	VulkanRenderCommandList* list;
    if(!free_command_lists.empty()) {
		list = free_command_lists.back();
		list->ResetCommandBuffer();
		free_command_lists.pop_back();
	} else {
		list = new VulkanRenderCommandList(shared_from_this());
	}


	return list->ResetSharedFromThis([](VulkanRenderCommandList* ptr) {
		auto vk_list = static_cast<VulkanRenderCommandList*>(ptr);
		auto destructible = static_cast<VulkanDeferredDestruction*>(vk_list);
		uint32_t value = vk_list->GetLastSubmitTimelineValue();
		if(value != 0) { // We only want to defer reuse or destruction if the buffer was submitted
			static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->AddToDeferredDestructionQueue(destructible, value);
		} else {
			destructible->Destroy();
		}
	});
}

VulkanRenderCommandAllocator::~VulkanRenderCommandAllocator()
{
	DEFINE_VK_INSTANCE(context);
	for(auto list : free_command_lists) {
		delete list;
	}
	vkDestroyCommandPool(context->GetVkDevice(), pool, NULL);
}

void VulkanRenderCommandAllocator::ReturnCommandList(VulkanRenderCommandList *list)
{
	std::lock_guard<std::mutex> lock(allocation_mutex);
	free_command_lists.push_back(list);
}
