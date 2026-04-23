#pragma once
#include <Renderer/RenderSurface.h>
#include "VulkanRenderResource.h"
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <Window.h>

class VulkanRenderPresentEvent : public RenderPresentEvent {

};

class VulkanRenderSurface : public RenderSurface {
public:

    VulkanRenderSurface(std::weak_ptr<Window> owning_window, VkSurfaceKHR surface, bool register_for_present = false);
    VulkanRenderSurface(const VulkanRenderSurface& ref) = delete;
    VulkanRenderSurface& operator=(const VulkanRenderSurface& ref) = delete;
    virtual ~VulkanRenderSurface();
    

    virtual std::shared_ptr<RenderFrameBufferResource> GetFrameBufferByIndex(int index) override;
    virtual std::shared_ptr<RenderFrameBufferResource> GetCurrentFrameBuffer() override;
    virtual int GetCurrentFramebufferIndex() override;

    void Present(RenderPresentEvent* event);

    void RegisterForPresent();

    bool RecreateSwapchain();

    bool CreateSwapchain();

    /**
     * Used to attemp swapchain recreation to make the surface valid after it went out of date.
     */
    bool TryValidateSurface();

    virtual bool IsSurfaceValid() override {
        return is_valid;
    };

    bool CheckSurfaceValidity();

private:

    VkSurfaceKHR vk_surface;
    vkb::Swapchain vkb_swapchain;
    VkSwapchainKHR vk_swapchain;
    std::vector<VkSemaphore> present_semaphores;
    std::vector<VkSemaphore> render_semaphores;
    std::vector<std::shared_ptr<RenderFrameBufferResource>> swapchain_framebuffers;
    std::unique_ptr<EventObserverBase> present_observer;
    VkFence swapchain_creation_fence;
    uint32_t current_index = 0, previous_index = 0;
    std::weak_ptr<Window> owning_window;
    bool is_valid = true;
};
