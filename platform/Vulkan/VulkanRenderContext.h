#pragma once
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <Renderer/RenderContext.h>

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


	VkInstance GetVkInstance() const { return vk_instance; }
	void SetSurface(VkSurfaceKHR surface) { vk_surface = surface; }
	void RequestExtension(const std::string& extension);
	void RequestExtensions(const char** extensions, int count);
	std::vector<const char*> GetExtensions();

protected:
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
};