#pragma once
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

class VulkanContext {
public:

	static void Create();
	void InitializeInstance();
	void InitializeDevice();
	static VulkanContext* Get();
	static void Shutdown();

	VkInstance GetVkInstance() const { return vk_instance; }
	void SetSurface(VkSurfaceKHR surface) { vk_surface = surface; }
	void RequestExtension(const std::string& extension);
	void RequestExtensions(const char** extensions, int count);
	std::vector<const char*> GetExtensions();

private:
	VulkanContext();
	std::vector<std::string> requested_extensions;
	virtual ~VulkanContext();
	VkInstance vk_instance;
	vkb::Instance vkb_instance;
	VkDevice vk_device;
	vkb::Device vkb_device;
	VkSurfaceKHR vk_surface;
	vkb::Swapchain vkb_swapchain;
	VkSwapchainKHR vk_swapchain;


	static VulkanContext* instance;
};