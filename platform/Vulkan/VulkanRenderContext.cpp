#include "VulkanRenderContext.h"
#include <VkBootstrap.h>
#include <stdexcept>
#include "VulkanRenderContext.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderResourceManager.h"
#include "VulkanRenderCommandList.h"
#include "Application.h"
#include "Window.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderSurface.h"
#include "dependencies/bullet/examples/ExampleBrowser/GwenGUISupport/GwenProfileWindow.h"

PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet_KHR = nullptr;

void VulkanRenderContext::Present()
{
	VulkanRenderPresentEvent event = {};

	Application::Get()->SendObservedEvent(&event);
}

uint64_t VulkanRenderContext::GetCurrentGpuTimelineValue()
{
	return static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue())->command_buffer_fence->GetValue();
}

uint64_t VulkanRenderContext::GetCurrentCpuTimelineValue()
{
	return static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue())->last_buffer_signaled;
}

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
	vkDeviceWaitIdle(vk_device);
	
	VulkanUnitConverter::Shutdown();

	vmaDestroyAllocator(allocator);

	delete Renderer::Get()->GetCommandQueue();
	SetRenderQueue(nullptr, RenderQueueTypes::DirectQueue);
	SetRenderQueue(nullptr, RenderQueueTypes::ComputeQueue);
	SetRenderQueue(nullptr, RenderQueueTypes::CopyQueue);

	vkb::destroy_device(vkb_device);
	vkb::destroy_instance(vkb_instance);
}

VulkanRenderContext::VulkanRenderContext() : requested_extensions(), vk_device(), 
	vkb_device(), vk_instance(), vkb_instance(),  allocator()
{
	requested_extensions.reserve(10);


}

void VulkanRenderContext::Init()
{

}

void VulkanRenderContext::PreInit()
{
	InstanceInit();

	vkb::PhysicalDeviceSelector selector(vkb_instance);
	selector.defer_surface_initialization(); // Window is not ready yet
	VkPhysicalDeviceVulkan12Features features_12 = {};
	features_12.bufferDeviceAddress = true;
	features_12.descriptorIndexing = true;
	features_12.timelineSemaphore = true;
	features_12.scalarBlockLayout = true;
	features_12.descriptorBindingVariableDescriptorCount = true;
	features_12.descriptorBindingSampledImageUpdateAfterBind = true;
	features_12.descriptorBindingStorageBufferUpdateAfterBind = true;
	features_12.descriptorBindingUniformBufferUpdateAfterBind = true;
	features_12.descriptorBindingPartiallyBound = true;
	features_12.descriptorBindingUpdateUnusedWhilePending = true;
	features_12.runtimeDescriptorArray = true;

	VkPhysicalDeviceVulkan13Features features_13 = {};
	features_13.dynamicRendering = true;
	features_13.synchronization2 = true;

	VkPhysicalDeviceFeatures features = {};
	features.geometryShader = true;
	features.depthClamp = true;

	VkPhysicalDeviceCustomBorderColorFeaturesEXT custom_sampler_border = {};
	custom_sampler_border.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
	custom_sampler_border.customBorderColors = true;
	custom_sampler_border.customBorderColorWithoutFormat = true;

	selector.add_required_extension_features(custom_sampler_border);
	
	selector.add_required_extension("VK_EXT_custom_border_color");
	selector.add_required_extension("VK_KHR_push_descriptor");
	selector.set_required_features_12(features_12);

	selector.set_required_features(features);
	selector.set_required_features_13(features_13);

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
	
	vkCmdPushDescriptorSet_KHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(vk_device,"vkCmdPushDescriptorSetKHR");

	VulkanRenderCommandQueue* queue = new VulkanRenderCommandQueue(vkb_device.get_queue(vkb::QueueType::graphics).value());

	SetRenderQueue(queue, RenderQueueTypes::DirectQueue);
	SetRenderQueue(queue, RenderQueueTypes::CopyQueue);
	SetRenderQueue(queue, RenderQueueTypes::ComputeQueue);

	VkPhysicalDeviceProperties2 properties = {};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &indexing_properties;

	indexing_properties = {};
	indexing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;

	vkGetPhysicalDeviceProperties2(device.value().physical_device, &properties);

	bindless_limits.max_bindless_storage_buffers = std::min(indexing_properties.maxDescriptorSetUpdateAfterBindStorageBuffers,
		indexing_properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers);

	bindless_limits.max_bindless_uniform_buffers = std::min(indexing_properties.maxDescriptorSetUpdateAfterBindUniformBuffers,
		indexing_properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers);

	bindless_limits.max_bindless_textures = std::min(indexing_properties.maxDescriptorSetUpdateAfterBindSampledImages,
		indexing_properties.maxPerStageDescriptorUpdateAfterBindSampledImages);

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.device = vk_device;
	allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocator_info.instance = vk_instance;
	allocator_info.physicalDevice = device.value();
	allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;

	vmaCreateAllocator(&allocator_info, &allocator);
	VulkanUnitConverter::Init();
}

void VulkanRenderContext::StartShutdown()
{
}

VulkanRenderContext::~VulkanRenderContext()
{

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
