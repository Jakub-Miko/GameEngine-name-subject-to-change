#pragma once
#include <Renderer/RenderCommandQueue.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <mutex>
#include <atomic>

class VulkanRenderCommandQueue : public RenderCommandQueue {
public:
	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;
	virtual void ExecuteRenderCommandList(std::shared_ptr<RenderCommandList> list) override;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) override;
	void WaitForValue(uint32_t value);

	void VkBinarySemaphoreSignal(VkSemaphore semaphore);
	void VkBinarySemaphoreWait(VkSemaphore semaphore, VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	VulkanRenderCommandQueue(VkQueue queue);

	VkQueue* GetVkQueue() { return &vk_queue; }

	virtual ~VulkanRenderCommandQueue() {};

private:
	friend class VulkanRenderContext;
	VkQueue vk_queue;
	std::mutex submit_mutex;
	std::atomic<uint64_t> last_buffer_signaled = 1; // We start with one because 0 needs to always be finished.
	std::shared_ptr<RenderFence> command_buffer_fence;
}; 