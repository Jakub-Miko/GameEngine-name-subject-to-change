#pragma once
#include <Renderer/RenderSurface.h>
#include "VulkanRenderResource.h"
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

class VulkanRenderPresentEvent : public RenderPresentEvent {
public:
    std::vector<VkSemaphore> render_finished_semaphores;
};

class VulkanRenderSurface : public RenderSurface {
public:

    VulkanRenderSurface(VkSurfaceKHR surface, bool register_for_present = false);
    VulkanRenderSurface(const VulkanRenderSurface& ref) = delete;
    VulkanRenderSurface& operator=(const VulkanRenderSurface& ref) = delete;
    ~VulkanRenderSurface();
    

    virtual std::shared_ptr<RenderFrameBufferResource> GetFrameBufferByIndex(int index) override;
    virtual std::shared_ptr<RenderFrameBufferResource> GetCurrentFrameBuffer() override;
    virtual int GetCurrentFramebufferIndex() override;

    void Present(RenderPresentEvent* event);

    void RegisterForPresent();

private:

    VkSurfaceKHR vk_surface;
    vkb::Swapchain vkb_swapchain;
    VkSwapchainKHR vk_swapchain;
    std::vector<VkSemaphore> present_semaphores;
    std::vector<VkSemaphore> render_semaphores;
    std::vector<std::shared_ptr<RenderFrameBufferResource>> swapchain_framebuffers;
    std::unique_ptr<EventObserverBase> present_observer;
    uint32_t current_index = 0;
};
