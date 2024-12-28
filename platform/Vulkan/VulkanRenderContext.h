#pragma once
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <Renderer/RenderContext.h>
#include <Core/FrameMultiBufferResource.h>
#define VMA_VULKAN_VERSION 1003000 
#include <vk_mem_alloc.h>

#define DEFINE_VK_INSTANCE(x) auto x = static_cast<VulkanRenderContext*>(RenderContext::Get());

class VulkanRenderContext : public RenderContext {
public:

	virtual void Init() override;
	virtual void PreInit() override;
	virtual void StartShutdown()override;
	virtual ~VulkanRenderContext() override;
	void InstanceInit();

	VulkanRenderContext(const VulkanRenderContext& ref) = delete;
	VulkanRenderContext(VulkanRenderContext&& ref) = delete;
	VulkanRenderContext& operator=(const VulkanRenderContext& ref) = delete;
	VulkanRenderContext& operator=(VulkanRenderContext&& ref) = delete;

	/**
	 * @brief Insert a wait in the queue to wait until a new render image is available, then set a new current framebuffer
	 * @note this is done after the presenting of the last frame so the resources of the next frame are used
	 */
	void StartNewFrame();



	/**
	 * @brief This Insert synchronization to finish the rendering before presenting
	 */
	void SignalEndFrame();

	VmaAllocator& GetVmaAllocator() { return allocator;  }
	uint32_t GetCurrentFramebufferIndex() const { return current_framebuffer; }
	VkInstance GetVkInstance() const { return vk_instance; }
	VkSwapchainKHR* GetVkSwapchain() { return &vk_swapchain; }
	VkDevice GetVkDevice() const { return vk_device; }
	vkb::Device GetVkbDevice() const { return vkb_device; }
	void SetSurface(VkSurfaceKHR surface) { vk_surface = surface; }
	VkSemaphore GetVkRenderSemaphore() { return frame_sync.render_fence.GetResource(); };
	void RequestExtension(const std::string& extension);
	void RequestExtensions(const char** extensions, int count);
	std::vector<const char*> GetExtensions();

protected:
	uint32_t GetNextPresentImageIndex();
	virtual void Destroy() override;

private:
	VulkanRenderContext();
	friend RenderContext;
	std::vector<std::string> requested_extensions;
	VkInstance vk_instance;
	vkb::Instance vkb_instance;
	VkDevice vk_device;
	vkb::Device vkb_device;
	VkSurfaceKHR vk_surface;
	vkb::Swapchain vkb_swapchain;
	VkSwapchainKHR vk_swapchain;
	uint32_t current_framebuffer;
	VmaAllocator allocator;
	struct {
		FrameMultiBufferResource<VkSemaphore> render_fence;
		FrameMultiBufferResource<VkSemaphore> present_fence; ///< we normally use timeline semaphores instead of fences, but vkAcquireNextImageKHR only takes binary ones
	} frame_sync;
};