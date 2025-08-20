#pragma once
#include <Renderer/RenderCommandQueue.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <mutex>
#include <atomic>

class VulkanRenderCommandQueue : public RenderCommandQueue {
public:
	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;
	virtual void ExecuteRenderCommandList(RenderCommandList* list) override;
	void ExecuteRenderCommandListWithSemaphores(RenderCommandList* list, const std::vector<VkSemaphore>& signal_sems, const std::vector<VkSemaphore>& wait_sems);

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) override;
	void WaitForValue(uint32_t value);

	void VkBinarySemaphoreSignal(VkSemaphore semaphore);
	void VkBinarySemaphoreWait(VkSemaphore semaphore, VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	VulkanRenderCommandQueue(VkQueue queue);

	VkQueue* GetVkQueue() { return &vk_queue; }

	virtual ~VulkanRenderCommandQueue() {};

private:
	friend class VulkanRenderContext;
	VkQueue vk_queue;
	std::mutex submit_mutex;
	std::atomic<uint64_t> last_buffer_signaled = 0;
	std::shared_ptr<RenderFence> command_buffer_fence;
}; 