#include "VulkanRenderContext.h"
#include <VkBootstrap.h>
#include <stdexcept>
#include "VulkanRenderContext.h"


void VulkanRenderContext::RequestExtension(const std::string& extension)
{
	auto fnd = std::find(requested_extensions.begin(), requested_extensions.end(), extension);
	if (fnd == requested_extensions.end()) {
		requested_extensions.push_back(extension);
	}
}

void VulkanRenderContext::RequestExtensions(const char** extensions, int count)
{
	for (int i = 0; i < count; i++) {
		RequestExtension(std::string(extensions[i]));
	}
}

std::vector<const char*> VulkanRenderContext::GetExtensions()
{
	std::vector<const char*> vec;

	int i = 0;
	for (auto& string : requested_extensions) {
		vec.push_back(string.c_str());
	}

	return std::move(vec);
}

void VulkanRenderContext::Destroy()
{
}

VulkanRenderContext::VulkanRenderContext() : requested_extensions(), vk_device(), 
	vkb_device(), vkb_swapchain(), vk_swapchain(), vk_instance(), vkb_instance(), vk_surface()
{
	requested_extensions.reserve(10);


}

void VulkanRenderContext::Init()
{
	vkb::PhysicalDeviceSelector selector(vkb_instance);
	selector.set_surface(vk_surface);

	auto device = selector.select();

	if (!device.has_value()) {
		throw std::runtime_error(device.error().message());
	}

	vkb::DeviceBuilder builder(device.value());
	auto device_result = builder.build();
	if (!device_result.has_value()) {
		throw std::runtime_error(device_result.error().message());
	}
	vkb_device = device_result.value();
	vk_device = vkb_device.device;

	vkb::SwapchainBuilder swapchain_builder(vkb_device);
	auto swapchain_result = swapchain_builder.build();
	if (!swapchain_result.has_value()) {
		throw std::runtime_error(swapchain_result.error().message());
	}
	vkb_swapchain = swapchain_result.value();
	vk_swapchain = vkb_swapchain.swapchain;
}

void VulkanRenderContext::PreInit()
{

}

void VulkanRenderContext::StartShutdown()
{
}

VulkanRenderContext::~VulkanRenderContext()
{
	vkb::destroy_swapchain(vkb_swapchain);
	vkb::destroy_device(vkb_device);
	vkb::destroy_instance(vkb_instance);
}

void VulkanRenderContext::InstanceInit()
{
	vkb::InstanceBuilder builder;
#ifndef NDEBUG
	builder.request_validation_layers();
#endif
	builder.use_default_debug_messenger();
	builder.set_app_name("NSTC Engine");
	builder.set_engine_name("NSTC Engine");
	builder.require_api_version(1, 3, 0);
	builder.enable_extensions(GetExtensions());
	vkb_instance = builder.build().value();
	vk_instance = vkb_instance.instance;
}
