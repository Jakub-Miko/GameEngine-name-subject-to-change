#pragma once
#include <Renderer/RenderCommandAllocator.h>
#include <Renderer/RenderCommandList.h>
#include <vulkan/vulkan.h>

class VulkanRenderCommandList;

class VulkanRenderCommandAllocator  : public RenderCommandAllocator, public std::enable_shared_from_this<VulkanRenderCommandAllocator> {
public:
	virtual void clear() override;

	VulkanRenderCommandAllocator(size_t starting_size);
	
	virtual std::shared_ptr<RenderCommandList> GetCommandList() override;

	virtual ~VulkanRenderCommandAllocator();

	VkCommandPool GetCommandPool() const {
		return pool;
	}

	void ReturnCommandList(VulkanRenderCommandList* list);

private:
	std::vector<VulkanRenderCommandList*> free_command_lists;
	std::mutex allocation_mutex;
	VkCommandPool pool;
};
