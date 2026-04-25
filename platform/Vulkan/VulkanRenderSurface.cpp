#include "VulkanRenderSurface.h"
#include "VulkanRenderResourceManager.h"
#include "Application.h"
#include "ConfigManager.h"
#include "VulkanRenderCommandQueue.h"

VulkanRenderSurface::VulkanRenderSurface(std::weak_ptr<Window> owning_window, VkSurfaceKHR surface, bool register_for_present)
    : vk_surface(surface), vkb_swapchain(), vk_swapchain(), present_semaphores(), swapchain_framebuffers(), present_observer(nullptr), owning_window(owning_window)
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
	present_semaphore_info.flags = 0;
	
	VkSemaphoreTypeCreateInfo render_semaphore_type_info;
	render_semaphore_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	render_semaphore_type_info.initialValue = 0;
	render_semaphore_type_info.pNext = NULL;
	render_semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
	
	VkSemaphoreCreateInfo render_semaphore_info;
	render_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	render_semaphore_info.pNext = &render_semaphore_type_info;
	render_semaphore_info.flags = 0;
	
	VkFence fence;
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence(context->GetVkDevice(), &info,NULL, &swapchain_creation_fence);

	CreateSwapchain();


	for(int i = 0; i < swapchain_framebuffers.size(); i++) {
		VkSemaphore render_semaphore;
		vkCreateSemaphore(vk_device, &render_semaphore_info, NULL, &render_semaphore);
        render_semaphores.push_back(render_semaphore);
	}


	for(int i = 0; i < swapchain_framebuffers.size(); i++) { // we need an extra semaphore for the initial image which we dont know the index of 
		VkSemaphore present_semaphore;
		vkCreateSemaphore(vk_device, &present_semaphore_info, NULL, &present_semaphore);
        present_semaphores.push_back(present_semaphore);
	}

    if(register_for_present) 
    {
        RegisterForPresent();
    }

}


VulkanRenderSurface::~VulkanRenderSurface()
{
    DEFINE_VK_INSTANCE(context);
	vkDeviceWaitIdle(context->GetVkDevice());
    for(auto sem : present_semaphores) {
        vkDestroySemaphore(context->GetVkDevice(), sem, NULL);
    }

	for(auto sem : render_semaphores) {
        vkDestroySemaphore(context->GetVkDevice(), sem, NULL);
    }

	vkDestroyFence(context->GetVkDevice(), swapchain_creation_fence, NULL);

	std::vector<VkImageView> views;
	views.reserve(swapchain_framebuffers.size());
	for(auto framebuf : swapchain_framebuffers) {
		views.push_back(static_cast<VulkanRenderTextureResource*>(framebuf->GetBufferDescriptor().color_attachments[0].resource->GetExtensionData())->GetImageView());
	}

	vkb_swapchain.destroy_image_views(views);

    vkb::destroy_swapchain(vkb_swapchain);
	vkb::destroy_surface(context->GetVkbInstance(), vk_surface);
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
    return is_valid ? swapchain_framebuffers[current_index] : nullptr;
}

int VulkanRenderSurface::GetCurrentFramebufferIndex()
{
    return current_index;
}

void VulkanRenderSurface::Present(RenderPresentEvent *event)
{
	if (!is_valid) { // Try surface revalidation to prepare the swapchain for the next frame.
		is_valid = TryValidateSurface();
		return;
	}

    auto queue  = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
    auto vk_present_info = static_cast<VulkanRenderPresentEvent*>(event);
    DEFINE_VK_INSTANCE(context);
    auto vk_device = context->GetVkDevice();    


	auto frame_buf = std::static_pointer_cast<VulkanRenderFrameBufferResource>(GetCurrentFrameBuffer());
	auto list_1 = std::static_pointer_cast<VulkanRenderCommandList>(Renderer::Get()->GetRenderCommandList());
	auto vk_command_buffer = list_1->GetVkCommandBuffer();
	auto manager = static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get());
	

	list_1->SetResourceDefaultState(frame_buf->GetBufferDescriptor().color_attachments[0].resource, RenderState::TEXTURE_PRESENT);

	queue->ExecuteRenderCommandList(list_1);

	queue->VkBinarySemaphoreSignal(render_semaphores[current_index]);
	
    
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_semaphores[current_index];
    info.swapchainCount = 1;
    info.pSwapchains = &vk_swapchain;
    info.pImageIndices = &current_index;
	
	auto& mutex = queue->GetQueueMutex();
	mutex.lock();
    vkQueuePresentKHR(*queue->GetVkQueue(), &info);
	mutex.unlock();

	previous_index = current_index;
	auto acquire_result = vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000,present_semaphores[current_index], NULL, &current_index); // timeout 30 seconds
	if (acquire_result == VK_SUBOPTIMAL_KHR) {
		RecreateSwapchain();
	}
	else if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		is_valid = false;
		return;
	}
	else {
		queue->VkBinarySemaphoreWait(present_semaphores[previous_index]); //Waits until the image is available so rendering can begin on it 
	}
	
	auto list_2 = std::static_pointer_cast<VulkanRenderCommandList>(Renderer::Get()->GetRenderCommandList());

	auto attachment = swapchain_framebuffers[current_index]->GetBufferDescriptor().color_attachments[0].resource;

	list_2->SetRenderTarget(swapchain_framebuffers[current_index]);
	list_2->SetResourceDefaultState(attachment, RenderState::TEXTURE_COLOR_ATTACHMENT);
	list_2->Clear();

	queue->ExecuteRenderCommandList(list_2);
}

bool VulkanRenderSurface::CreateSwapchain() {
	DEFINE_VK_INSTANCE(context);
    auto vkb_device = context->GetVkbDevice();
    auto vk_device = context->GetVkDevice();
	auto v_sync = ConfigManager::Get()->GetInt("Vsync") == 1;
	auto queue  = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());

	vkb::SwapchainBuilder swapchain_builder(vkb_device,vk_surface);
	swapchain_builder.add_image_usage_flags(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	swapchain_builder.set_desired_min_image_count(FrameManager::Get()->GetLatencyFrames());
	swapchain_builder.set_desired_present_mode(v_sync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR);

	if(vkb_swapchain) {
		swapchain_builder.set_old_swapchain(vkb_swapchain);
	}
	auto swapchain_result = swapchain_builder.build();
	if (!swapchain_result.has_value()) {
		throw std::runtime_error(swapchain_result.error().message());
	}

	vkb_swapchain = swapchain_result.value();
	vk_swapchain = vkb_swapchain.swapchain;

	auto code = vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000,NULL, swapchain_creation_fence, &current_index); // timeout 30 seconds
	if (code == VK_ERROR_OUT_OF_DATE_KHR) {
		return false;
	}

	auto extent = vkb_swapchain.extent;
	auto images = vkb_swapchain.get_images().value();
	auto views = vkb_swapchain.get_image_views().value();

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
	vkWaitForFences(context->GetVkDevice(),1, &swapchain_creation_fence, VK_TRUE, 30000000000);
	vkResetFences(context->GetVkDevice(), 1, &swapchain_creation_fence);
	vkQueueWaitIdle(*queue->GetVkQueue());
	return true;
}

bool VulkanRenderSurface::TryValidateSurface()
{
	auto queue = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	if (CheckSurfaceValidity() && RecreateSwapchain()) { // If this surface was previously invalid and should now be valid recreate the swapchain.
		auto list_2 = std::static_pointer_cast<VulkanRenderCommandList>(Renderer::Get()->GetRenderCommandList());

		auto attachment = swapchain_framebuffers[current_index]->GetBufferDescriptor().color_attachments[0].resource;

		list_2->SetRenderTarget(swapchain_framebuffers[current_index]);
		list_2->SetResourceDefaultState(attachment, RenderState::TEXTURE_COLOR_ATTACHMENT);
		list_2->Clear();

		queue->ExecuteRenderCommandList(list_2);
		return true;
	}
	else {
		return false;
	}
}

bool VulkanRenderSurface::CheckSurfaceValidity()
{
	if (auto window = owning_window.lock()) {
		bool is_minimized = window->IsMinimized();
		auto framebuffer_res = window->GetFramebufferResolution();
		return !is_minimized && framebuffer_res.x != 0 && framebuffer_res.y != 0;
	}
	else {
		return false;
	}
}

bool VulkanRenderSurface::RecreateSwapchain() {
	swapchain_framebuffers.clear();
	current_index = 0;
	previous_index = 0;
	return CreateSwapchain();
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
