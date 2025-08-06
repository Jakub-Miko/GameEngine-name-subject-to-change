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

PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet_KHR = nullptr;

void VulkanRenderContext::StartNewFrame()
{
	GetNextPresentImageIndex(); // Get the a swapchain image index for the upcoming frame, this signals the present_fence of the next image after the image becomes available
	auto vulkan_queue = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	vulkan_queue->VkBinarySemaphoreWait(frame_sync.present_fence[previous_framebuffer]); //Waits until the image is available so rendering can begin on it 
	auto list = static_cast<VulkanRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
	auto vk_command_buffer = list->GetVkCommandBuffer();

	auto attachment = static_cast<VulkanRenderTextureResource*>(default_framebuffers[current_framebuffer]->GetBufferDescriptor().color_attachments[0].resource->GetExtensionData());
	VkImageSubresourceRange range;
	range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.layerCount = 1;

	VkImageMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
	barrier.srcAccessMask = VK_ACCESS_2_NONE;
	barrier.dstAccessMask = VK_ACCESS_2_NONE;
	barrier.image = attachment->GetImage();
	barrier.subresourceRange = range;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.imageMemoryBarrierCount = 1;
	info.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(*vk_command_buffer, &info);
	
	

	list->SetDefaultRenderTarget();
	list->Clear();

	vulkan_queue->ExecuteRenderCommandList(list);
}

void VulkanRenderContext::SignalEndFrame()
{
	auto vulkan_queue = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	auto list = static_cast<VulkanRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
	auto vk_command_buffer = list->GetVkCommandBuffer();

	auto attachment = static_cast<VulkanRenderTextureResource*>(default_framebuffers[current_framebuffer]->GetBufferDescriptor().color_attachments[0].resource->GetExtensionData());
	VkImageSubresourceRange range;
	range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.layerCount = 1;


	VkImageMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
	barrier.srcAccessMask = VK_ACCESS_2_NONE;
	barrier.dstAccessMask = VK_ACCESS_2_NONE;
	barrier.image = attachment->GetImage();
	barrier.subresourceRange = range;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.imageMemoryBarrierCount = 1;
	info.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(*vk_command_buffer, &info);

	vulkan_queue->ExecuteRenderCommandList(list);
	vulkan_queue->VkBinarySemaphoreSignal(frame_sync.render_fence[current_framebuffer]);

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

uint32_t VulkanRenderContext::GetNextPresentImageIndex()
{
	previous_framebuffer = current_framebuffer;
	auto code = vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000,frame_sync.present_fence[current_framebuffer], NULL, &current_framebuffer); // timeout 30 seconds
	if (code == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000, frame_sync.present_fence[current_framebuffer], NULL, &current_framebuffer);
	}
	return current_framebuffer;
}

void VulkanRenderContext::CreateSwapchain()
{
	VkSemaphoreTypeCreateInfo present_semaphore_type_info;
	present_semaphore_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	present_semaphore_type_info.initialValue = 0;
	present_semaphore_type_info.pNext = NULL;
	present_semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;

	VkSemaphoreCreateInfo present_semaphore_info;
	present_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	present_semaphore_info.pNext = &present_semaphore_type_info;
	present_semaphore_info.flags = NULL;

	vkb::SwapchainBuilder swapchain_builder(vkb_device);
	swapchain_builder.add_image_usage_flags(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	swapchain_builder.set_desired_min_image_count(FrameManager::Get()->GetLatencyFrames());
	auto swapchain_result = swapchain_builder.build();
	if (!swapchain_result.has_value()) {
		throw std::runtime_error(swapchain_result.error().message());
	}
	vkb_swapchain = swapchain_result.value();
	vk_swapchain = vkb_swapchain.swapchain;
	
	auto extent = vkb_swapchain.extent;
	auto images = vkb_swapchain.get_images().value();
	auto views = vkb_swapchain.get_image_views().value();
	
	VkSemaphoreTypeCreateInfo render_semaphore_type_info;
	render_semaphore_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	render_semaphore_type_info.initialValue = 0;
	render_semaphore_type_info.pNext = NULL;
	render_semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;

	VkSemaphoreCreateInfo render_semaphore_info;
	render_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	render_semaphore_info.pNext = &render_semaphore_type_info;
	render_semaphore_info.flags = NULL;
	
	for(int i = 0; i < images.size(); i++) {
		VkSemaphore present_fence;
		vkCreateSemaphore(vk_device, &present_semaphore_info, NULL, &present_fence);
		frame_sync.present_fence.push_back(present_fence);

		VkSemaphore render_fence;
		vkCreateSemaphore(vk_device, &render_semaphore_info, NULL, &render_fence);
		frame_sync.render_fence.push_back(render_fence);
	}

	auto resource_manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	auto props = Application::Get()->GetWindow()->GetProperties();

	RenderTexture2DDescriptor swapchain_image_desc;
	swapchain_image_desc.format = TextureFormat::UNDEFINED; // This image is never accesed by the user so the descriptor contents are not important 
	swapchain_image_desc.height = extent.height;
	swapchain_image_desc.width = extent.width;
	swapchain_image_desc.sampler = nullptr;
	swapchain_image_desc.usage = TextureUsage::COLOR_ATTACHMENT;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEFAULT_DEPTH;
	depth_desc.height = props.resolution_y;
	depth_desc.width = props.resolution_x;
	depth_desc.sampler = nullptr;
	depth_desc.usage = TextureUsage::DEPTH_ATTACHMENT;

	std::vector<RenderFrameBufferDescriptor::RenderFrameBufferAttachment> swapchain_images;

	swapchain_images.reserve(images.size());
	int swapchain_size = images.size();
	for (int i = 0; i < swapchain_size; i++) {
		auto image = images[i];
		auto view = views[i];

		RenderTexture2DResource* texture = resource_manager->CreateNonManagedTexture(image, view, swapchain_image_desc, RenderState::TEXTURE_COLOR_ATTACHMENT);
		std::shared_ptr<RenderTexture2DResource> color_texture = std::shared_ptr<RenderTexture2DResource>(texture);
		RenderFrameBufferDescriptor::RenderFrameBufferAttachment attachment;
		attachment.level = 0;
		attachment.resource = color_texture;
		auto depth_buffer = resource_manager->CreateTexture(depth_desc, RenderState::TEXTURE_DEPTH_STENCIL_ATTACHMENT);
		RenderFrameBufferDescriptor default_framebuf;
		default_framebuf.color_attachments = { {0, color_texture } };
		default_framebuf.depth_stencil_attachment = { 0, depth_buffer };
		auto framebuffer = resource_manager->CreateFrameBuffer(default_framebuf);
		default_framebuffers.push_back(framebuffer);
	}

}

void VulkanRenderContext::RecreateSwapchain()
{
	for (auto& ref : frame_sync.present_fence) {
		vkDestroySemaphore(vk_device, ref, NULL);
	}

	for (auto& ref : frame_sync.render_fence) {
		vkDestroySemaphore(vk_device, ref, NULL);
	}

	frame_sync.present_fence.clear();
	frame_sync.render_fence.clear();

	vkb::destroy_swapchain(vkb_swapchain);

	default_framebuffers.clear();

	CreateSwapchain();
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
	VulkanUnitConverter::Shutdown();
	for (auto& ref : frame_sync.present_fence) {
		vkDestroySemaphore(vk_device, ref, NULL);
	}

	frame_sync.present_fence.clear();

	for (auto& ref : frame_sync.render_fence) {
		vkDestroySemaphore(vk_device, ref, NULL);
	}

	frame_sync.render_fence.clear();

	vmaDestroyAllocator(allocator);

	delete Renderer::Get()->GetCommandQueue();
	SetRenderQueue(nullptr, RenderQueueTypes::DirectQueue);
	SetRenderQueue(nullptr, RenderQueueTypes::ComputeQueue);
	SetRenderQueue(nullptr, RenderQueueTypes::CopyQueue);

	vkb::destroy_swapchain(vkb_swapchain);
	vkb::destroy_device(vkb_device);
	vkb::destroy_instance(vkb_instance);
}

VulkanRenderContext::VulkanRenderContext() : requested_extensions(), vk_device(), 
	vkb_device(), vkb_swapchain(), vk_swapchain(), vk_instance(), vkb_instance(), vk_surface(), allocator()
{
	requested_extensions.reserve(10);


}

void VulkanRenderContext::Init()
{
	vkb::PhysicalDeviceSelector selector(vkb_instance);
	selector.set_surface(vk_surface);
	VkPhysicalDeviceVulkan12Features features_12 = {};
	features_12.bufferDeviceAddress = true;
	features_12.descriptorIndexing = true;
	features_12.timelineSemaphore = true;
	features_12.scalarBlockLayout = true;

	VkPhysicalDeviceVulkan13Features features_13 = {};
	features_13.dynamicRendering = true;
	features_13.synchronization2 = true;

	VkPhysicalDeviceFeatures features = {};
	features.geometryShader = true;

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



	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.device = vk_device;
	allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocator_info.instance = vk_instance;
	allocator_info.physicalDevice = device.value();
	allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;

	vmaCreateAllocator(&allocator_info, &allocator);
	VulkanUnitConverter::Init();

	CreateSwapchain();

	StartNewFrame();

}

void VulkanRenderContext::PreInit()
{

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
