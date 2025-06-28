#pragma once
#include <Renderer/RenderFence.h>
#include <mutex>
#include <cstddef>
#include <Core/ExecutableCommand.h>
#include <condition_variable>
#include <vulkan/vulkan.h>

class VulkanRenderFence : public RenderFence {
public:
	friend class VulkanRenderCommandQueue;
	friend class VulkanRenderFenceCommand;
	friend class RenderFence;
	virtual bool WaitForValue(int desired_value) override;
	virtual void Wait() override;
	virtual int GetValue() override;


	VkSemaphore* GetSemaphore() { return &semaphore; }
private:
	VulkanRenderFence();
	virtual ~VulkanRenderFence();

	VulkanRenderFence(const VulkanRenderFence& ref) = delete;
	VulkanRenderFence(VulkanRenderFence&& ref) = delete;
	VulkanRenderFence& operator=(const VulkanRenderFence& ref) = delete;
	VulkanRenderFence& operator=(VulkanRenderFence&& ref) = delete;

	/**
	 * @brief Signals fence from host
	 * @warning unlike the OpenGL version, this version is host signal, not gpu signal;
	 */
	void Signal(int num); 

	VkSemaphore* GetVkSemaphore() { return &semaphore; }
private:
	VkSemaphore semaphore; // Vulkan timeline semaphore behaves more like DirectX fence;
};
