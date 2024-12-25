#pragma once
#include <Renderer/RenderCommandQueue.h>
#include <vulkan/vulkan.h>

class VulkanRenderCommandQueue : public RenderCommandQueue {
public:
	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;
	virtual void ExecuteRenderCommandList(RenderCommandList* list) override;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) override;
	virtual void Present() override;

	VulkanRenderCommandQueue(VkQueue queue) : vk_queue(queue) {}

	VkQueue* GetVkQueue() { return &vk_queue; }

	virtual ~VulkanRenderCommandQueue() {};

private:
	VkQueue vk_queue;
}; 