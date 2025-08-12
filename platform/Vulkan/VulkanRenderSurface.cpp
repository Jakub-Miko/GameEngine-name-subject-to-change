#include "VulkanRenderSurface.h"
#include "VulkanRenderResourceManager.h"
#include "Application.h"
#include "VulkanRenderCommandQueue.h"

VulkanRenderSurface::VulkanRenderSurface(VkSurfaceKHR surface, bool register_for_present) 
    : vk_surface(surface), vkb_swapchain(), vk_swapchain(), present_semaphores(), swapchain_framebuffers(), present_observer(nullptr)
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
	
	VkSemaphoreTypeCreateInfo render_semaphore_type_info;
	render_semaphore_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	render_semaphore_type_info.initialValue = 0;
	render_semaphore_type_info.pNext = NULL;
	render_semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
	
	VkSemaphoreCreateInfo render_semaphore_info;
	render_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	render_semaphore_info.pNext = &render_semaphore_type_info;
	render_semaphore_info.flags = NULL;
	
	VkSemaphore present_semaphore; // create at least one semaphore so the create swapchain function can use it to fetch the first image
	vkCreateSemaphore(vk_device, &present_semaphore_info, NULL, &present_semaphore);
    present_semaphores.push_back(present_semaphore);
	
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
	auto list_1 = static_cast<VulkanRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
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
	
    vkQueuePresentKHR(*queue->GetVkQueue(), &info);
	
	previous_index = current_index;
	auto code = vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000,present_semaphores[current_index + 1], NULL, &current_index); // timeout 30 seconds
	if (code == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000, present_semaphores[current_index + 1], NULL, &current_index);
	}
	
	queue->VkBinarySemaphoreWait(present_semaphores[previous_index + 1]); //Waits until the image is available so rendering can begin on it 
	auto list_2 = static_cast<VulkanRenderCommandList*>(Renderer::Get()->GetRenderCommandList());

	auto attachment = swapchain_framebuffers[current_index]->GetBufferDescriptor().color_attachments[0].resource;
	

	list_2->SetDefaultRenderTarget();
	list_2->SetResourceDefaultState(attachment, RenderState::TEXTURE_COLOR_ATTACHMENT);
	list_2->Clear();

	queue->ExecuteRenderCommandList(list_2);
}

void VulkanRenderSurface::CreateSwapchain() {
	 DEFINE_VK_INSTANCE(context);
    auto vkb_device = context->GetVkbDevice();
    auto vk_device = context->GetVkDevice();


	vkb::SwapchainBuilder swapchain_builder(vkb_device,vk_surface);
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

	auto code = vkAcquireNextImageKHR(vk_device, vk_swapchain, 30000000000,present_semaphores[0], NULL, &current_index); // timeout 30 seconds
	if (code == VK_ERROR_OUT_OF_DATE_KHR) {
		throw std::runtime_error{"Could not acquire the first swapchain image.\n"};
	}

	auto queue  = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->VkBinarySemaphoreWait(present_semaphores[0]); //Waits until the image is available so rendering can begin on it 
}

void VulkanRenderSurface::RecreateSwapchain() {
    vkb::destroy_swapchain(vkb_swapchain);
	swapchain_framebuffers.clear();
	current_index = 0;
	previous_index = 0;
	CreateSwapchain();
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
