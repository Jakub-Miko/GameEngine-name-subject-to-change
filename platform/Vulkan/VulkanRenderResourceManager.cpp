#include "VulkanRenderResourceManager.h"
#include "VulkanRenderContext.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderCommandQueue.h"

std::shared_ptr<RenderBufferResource> VulkanRenderResourceManager::CreateBuffer(const RenderBufferDescriptor& buffer_desc, RenderState default_state)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource(buffer_desc, default_state);

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = buffer_desc.buffer_size;
	info.usage = VulkanUnitConverter::BufferUsageToVkFlags(buffer_desc.usage);

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VulkanUnitConverter::BufferTypeToVmaUsage(buffer_desc.type);
	alloc_info.flags = VulkanUnitConverter::BufferTypeToVmaFlags(buffer_desc.type);

	vmaCreateBuffer(alloc, &info, &alloc_info, &new_buffer->buffer, &new_buffer->alloc, NULL);
	
	
	return std::shared_ptr<RenderBufferResource>(new_buffer, [](RenderBufferResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnBufferResource(static_cast<VulkanRenderBufferResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset)
{
}

void VulkanRenderResourceManager::ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size)
{
}

std::shared_ptr<RenderTexture2DResource> VulkanRenderResourceManager::CreateTexture(const RenderTexture2DDescriptor& buffer_desc, RenderState default_state)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();
	
	VkExtent3D extent;
	extent.depth = 1;
	extent.width = buffer_desc.width;
	extent.height = buffer_desc.height;

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.arrayLayers = 1;
	image_info.extent = extent;
	image_info.format = VulkanUnitConverter::TextureFormatToVulkanInternalformat(buffer_desc.format);
	image_info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	image_info.initialLayout = VulkanUnitConverter::RenderStateToTextureLayout(default_state);
	image_info.mipLevels = 1; /// @todo Add mipmap spec to descriptor;
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(buffer_desc.usage);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(alloc,&image_info,&alloc_info,&image,&allocation,NULL);
	
	VulkanRenderTexture2DResource* new_texture = new VulkanRenderTexture2DResource(buffer_desc, default_state);

	new_texture->alloc = allocation;
	new_texture->texture = image;

	return std::shared_ptr<RenderTexture2DResource>(new_texture, [](RenderTexture2DResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnTexture2DResource(static_cast<VulkanRenderTexture2DResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
}

void VulkanRenderResourceManager::GenerateMIPs(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource)
{
}

void VulkanRenderResourceManager::UploadToTexture2DFromFile(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level)
{
}

std::shared_ptr<RenderTexture2DResource> VulkanRenderResourceManager::CreateTextureFromFile(RenderCommandList* list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler)
{
	return std::shared_ptr<RenderTexture2DResource>();
}

std::shared_ptr<RenderTexture2DArrayResource> VulkanRenderResourceManager::CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderTexture2DArrayResource>();
}

void VulkanRenderResourceManager::UploadDataToTexture2DArray(RenderCommandList* list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
}

std::shared_ptr<RenderTexture2DCubemapResource> VulkanRenderResourceManager::CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderTexture2DCubemapResource>();
}

void VulkanRenderResourceManager::UploadDataToTexture2DCubemap(RenderCommandList* list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
}

std::shared_ptr<RenderFrameBufferResource> VulkanRenderResourceManager::CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderFrameBufferResource>();
}

void VulkanRenderResourceManager::CreateConstantBufferDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource)
{
}

void VulkanRenderResourceManager::CreateTexture2DDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource)
{
}

Future<read_pixel_data> VulkanRenderResourceManager::GetPixelValue(std::shared_ptr<RenderFrameBufferResource> framebuffer, int color_attachment_index, float x, float y)
{
	return Future<read_pixel_data>();
}

void VulkanRenderResourceManager::CreateTexture2DArrayDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource)
{
}

void VulkanRenderResourceManager::CreateTexture2DCubemapDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource)
{
}

void VulkanRenderResourceManager::CopyFrameBufferDepthAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer)
{
}

void VulkanRenderResourceManager::SetFrameBufferColorAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> framebuffer, std::shared_ptr<RenderResource> new_attachment, int index, int level)
{
}

VulkanRenderResourceManager::VulkanRenderResourceManager() : buffer_deletion_queue(), buffer_deletion_queue_mutex(), texture_deletion_queue(), texture_deletion_queue_mutex()
{
}

VulkanRenderResourceManager::~VulkanRenderResourceManager()
{
}

void VulkanRenderResourceManager::FlushDeletions()
{
	FlushBufferDeletions();
	FlushTextureDeletions();
}

void VulkanRenderResourceManager::FlushBufferDeletions()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	std::unique_lock<std::mutex> lock(buffer_deletion_queue_mutex);
	
	uint64_t current_timeline = context->GetCurrentGpuTimelineValue();
	
	VulkanRenderBufferResource* resource = nullptr;
	while ((resource = buffer_deletion_queue.front()) && resource->read_timeline < current_timeline && resource->write_timeline < current_timeline) {
		vmaDestroyBuffer(alloc, resource->buffer, resource->alloc);
		delete resource;
		buffer_deletion_queue.pop();
	}
}

void VulkanRenderResourceManager::FlushTextureDeletions()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	std::unique_lock<std::mutex> lock(texture_deletion_queue_mutex);

	uint64_t current_timeline = context->GetCurrentGpuTimelineValue();

	VulkanRenderTexture2DResource* resource = nullptr;
	while ((resource = texture_deletion_queue.front()) && resource->read_timeline < current_timeline && resource->write_timeline < current_timeline) { 
		vmaDestroyImage(alloc, resource->texture, resource->alloc);
		delete resource;
		texture_deletion_queue.pop();
	}
}

void VulkanRenderResourceManager::ReturnBufferResource(VulkanRenderBufferResource* resource)
{
	std::unique_lock<std::mutex> lock(buffer_deletion_queue_mutex);
	buffer_deletion_queue.push(resource);
}

void VulkanRenderResourceManager::ReturnTexture2DResource(VulkanRenderTexture2DResource* resource)
{
	std::unique_lock<std::mutex> lock(texture_deletion_queue_mutex);
	texture_deletion_queue.push(resource);
}

void VulkanRenderResourceManager::ReturnTexture2DArrayResource(RenderTexture2DArrayResource* resource)
{
}

void VulkanRenderResourceManager::ReturnFrameBufferResource(RenderFrameBufferResource* resource)
{
}

void VulkanRenderResourceManager::ReturnTexture2DCubemapResource(RenderTexture2DCubemapResource* resource)
{
}
