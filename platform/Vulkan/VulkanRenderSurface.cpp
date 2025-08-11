#include "VulkanRenderSurface.h"
#include "VulkanRenderResourceManager.h"
#include "Application.h"
#include "VulkanRenderCommandQueue.h"

VulkanRenderSurface::VulkanRenderSurface(VkSurfaceKHR surface, bool register_for_present) 
    : vk_surface(), vkb_swapchain(), vk_swapchain(), present_semaphores(), swapchain_framebuffers(), present_observer(nullptr)
{
    DEFINE_VK_INSTANCE(context);
    auto vkb_device = context->GetVkbDevice();
    auto vk_device = context->GetVkDevice();

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
		VkSemaphore present_semaphore;
		vkCreateSemaphore(vk_device, &present_semaphore_info, NULL, &present_semaphore);
        present_semaphores.push_back(present_semaphore);

		VkSemaphore render_semaphore;
		vkCreateSemaphore(vk_device, &present_semaphore_info, NULL, &render_semaphore);
        render_semaphores.push_back(render_semaphore);
	}

	auto resource_manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());

	RenderTexture2DDescriptor swapchain_image_desc;
	swapchain_image_desc.format = TextureFormat::UNDEFINED; // This image is never accesed by the user so the descriptor contents are not important 
	swapchain_image_desc.height = extent.height;
	swapchain_image_desc.width = extent.width;
	swapchain_image_desc.sampler = nullptr;
	swapchain_image_desc.usage = TextureUsage::COLOR_ATTACHMENT;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEFAULT_DEPTH;
	depth_desc.height = extent.height;
	depth_desc.width = extent.width;
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
		swapchain_framebuffers.push_back(framebuffer);
	}

    if(register_for_present) 
    {
        RegisterForPresent();
    }

}


VulkanRenderSurface::~VulkanRenderSurface()
{
    DEFINE_VK_INSTANCE(context);
    for(auto sem : present_semaphores) {
        vkDestroySemaphore(context->GetVkDevice(), sem, NULL);
    }

	for(auto sem : render_semaphores) {
        vkDestroySemaphore(context->GetVkDevice(), sem, NULL);
    }


    vkb::destroy_swapchain(vkb_swapchain);
}

std::shared_ptr<RenderFrameBufferResource> VulkanRenderSurface::GetFrameBufferByIndex(int index)
{
    if(index < 0 || index >= swapchain_framebuffers.size()) {
        throw std::runtime_error("Invalid FrameBuffer index passed to VulkanRenderSurface::GetFrameBufferByIndex.\n");
    }
    return swapchain_framebuffers[index];
}

std::shared_ptr<RenderFrameBufferResource> VulkanRenderSurface::GetCurrentFrameBuffer()
{
    return swapchain_framebuffers[current_index];
}

int VulkanRenderSurface::GetCurrentFramebufferIndex()
{
    return current_index;
}

void VulkanRenderSurface::Present(RenderPresentEvent *event)
{
    auto vk_present_info = static_cast<VulkanRenderPresentEvent*>(event);
    DEFINE_VK_INSTANCE(context);
    auto vk_device = context->GetVkDevice();    
    auto queue  = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());


	auto frame_buf = std::static_pointer_cast<VulkanRenderFrameBufferResource>(GetCurrentFrameBuffer());
	auto list = static_cast<VulkanRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
	auto vk_command_buffer = list->GetVkCommandBuffer();
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	
	

	auto attachment = static_cast<VulkanRenderTextureResource*>(frame_buf->GetBufferDescriptor().color_attachments[0].resource->GetExtensionData());
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

	list->SetResourceDefaultState(frame_buf->GetBufferDescriptor().color_attachments[0].resource, RenderState::TEXTURE_PRESENT);

	queue->ExecuteRenderCommandList(list);

	queue->VkBinarySemaphoreSignal(render_semaphores[current_index]);
	
    
	
    // VkPresentInfoKHR info = {};
    // info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // info.waitSemaphoreCount = vk_present_info->render_finished_semaphores.size();
    // info.pWaitSemaphores = vk_present_info->render_finished_semaphores.data();
    // info.swapchainCount = 1;
    // info.pSwapchains = &vk_swapchain;
    // info.pImageIndices = &current_index;
	
    // vkQueuePresentKHR(*queue->GetVkQueue(), &info);
	
	
	
	// GetNextPresentImageIndex(); // Get the a swapchain image index for the upcoming frame, this signals the present_fence of the next image after the image becomes available
	// auto vulkan_queue = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	// vulkan_queue->VkBinarySemaphoreWait(frame_sync.present_fence[previous_framebuffer]); //Waits until the image is available so rendering can begin on it 
	// auto list = static_cast<VulkanRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
	// auto vk_command_buffer = list->GetVkCommandBuffer();

	// auto attachment = static_cast<VulkanRenderTextureResource*>(default_framebuffers[current_framebuffer]->GetBufferDescriptor().color_attachments[0].resource->GetExtensionData());
	// VkImageSubresourceRange range;
	// range.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	// range.baseArrayLayer = 0;
	// range.baseMipLevel = 0;
	// range.levelCount = 1;
	// range.layerCount = 1;

	// VkImageMemoryBarrier2 barrier = {};
	// barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	// barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	// barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
	// barrier.srcAccessMask = VK_ACCESS_2_NONE;
	// barrier.dstAccessMask = VK_ACCESS_2_NONE;
	// barrier.image = attachment->GetImage();
	// barrier.subresourceRange = range;
	// barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	// VkDependencyInfo info = {};
	// info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	// info.imageMemoryBarrierCount = 1;
	// info.pImageMemoryBarriers = &barrier;

	// vkCmdPipelineBarrier2(*vk_command_buffer, &info);
	

	// list->SetDefaultRenderTarget();
	// list->Clear();

	// vulkan_queue->ExecuteRenderCommandList(list);
}

void VulkanRenderSurface::RegisterForPresent()
{
    if(!present_observer.get()) {
        present_observer.reset(MakeEventObserver<RenderPresentEvent>([this](RenderPresentEvent* event) {
			Present(event);
			return false;
		}));
        Application::Get()->RegisterObserver<RenderPresentEvent>(present_observer.get());
    }
}
