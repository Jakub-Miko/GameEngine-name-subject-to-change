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
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderBufferResource*>(resource));
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

	TextureUsage usage = buffer_desc.usage;
	if (usage == TextureUsage::DEFAULT) {
		usage = VulkanUnitConverter::TextureFormatToVulkanDefaultImageUsage(buffer_desc.format);
	}

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.arrayLayers = 1;
	image_info.extent = extent;
	image_info.format = VulkanUnitConverter::TextureFormatToVulkanInternalformat(buffer_desc.format);
	image_info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.mipLevels = 1; /// @todo Add mipmap spec to descriptor;
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(usage);
	

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(alloc,&image_info,&alloc_info,&image,&allocation,NULL);
	
	VulkanRenderTexture2DResource* new_texture = new VulkanRenderTexture2DResource(buffer_desc, default_state);

	new_texture->alloc = allocation;
	new_texture->image = image;

	return std::shared_ptr<RenderTexture2DResource>(new_texture, [](RenderTexture2DResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderTexture2DResource*>(resource));
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

std::shared_ptr<RenderTexture2DArrayResource> VulkanRenderResourceManager::CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc, RenderState default_state)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	VkExtent3D extent;
	extent.depth = 1;
	extent.width = buffer_desc.width;
	extent.height = buffer_desc.height;

	TextureUsage usage = buffer_desc.usage;
	if (usage == TextureUsage::DEFAULT) {
		usage = VulkanUnitConverter::TextureFormatToVulkanDefaultImageUsage(buffer_desc.format);
	}

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.arrayLayers = buffer_desc.num_of_textures;
	image_info.extent = extent;
	image_info.format = VulkanUnitConverter::TextureFormatToVulkanInternalformat(buffer_desc.format);
	image_info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.mipLevels = 1; /// @todo Add mipmap spec to descriptor;
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(usage);


	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(alloc, &image_info, &alloc_info, &image, &allocation, NULL);

	VulkanRenderTexture2DArrayResource* new_texture = new VulkanRenderTexture2DArrayResource(buffer_desc, default_state);

	new_texture->alloc = allocation;
	new_texture->image = image;

	return std::shared_ptr<RenderTexture2DArrayResource>(new_texture, [](RenderTexture2DArrayResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderTexture2DArrayResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToTexture2DArray(RenderCommandList* list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
}

std::shared_ptr<RenderTexture2DCubemapResource> VulkanRenderResourceManager::CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc, RenderState default_state)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	VkExtent3D extent;
	extent.depth = 1;
	extent.width = buffer_desc.res;
	extent.height = buffer_desc.res;

	TextureUsage usage = buffer_desc.usage;
	if (usage == TextureUsage::DEFAULT) {
		usage = VulkanUnitConverter::TextureFormatToVulkanDefaultImageUsage(buffer_desc.format);
	}

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.arrayLayers = 6;
	image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	image_info.extent = extent;
	image_info.format = VulkanUnitConverter::TextureFormatToVulkanInternalformat(buffer_desc.format);
	image_info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.mipLevels = 1; /// @todo Add mipmap spec to descriptor;
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(usage);


	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VmaAllocation allocation;

	vmaCreateImage(alloc, &image_info, &alloc_info, &image, &allocation, NULL);

	VulkanRenderTexture2DCubemapResource* new_texture = new VulkanRenderTexture2DCubemapResource(buffer_desc, default_state);

	new_texture->alloc = allocation;
	new_texture->image = image;

	return std::shared_ptr<RenderTexture2DCubemapResource>(new_texture, [](RenderTexture2DCubemapResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderTexture2DCubemapResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToTexture2DCubemap(RenderCommandList* list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
}

std::shared_ptr<RenderFrameBufferResource> VulkanRenderResourceManager::CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderFrameBufferResource>(new VulkanRenderFrameBufferResource(buffer_desc));
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

VulkanRenderResourceManager::VulkanRenderResourceManager() : deletion_queue(), deletion_queue_mutex()
{
}

VulkanRenderResourceManager::~VulkanRenderResourceManager()
{
}

void VulkanRenderResourceManager::FlushDeletions()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	std::unique_lock<std::mutex> lock(deletion_queue_mutex);

	uint64_t current_timeline = context->GetCurrentGpuTimelineValue();

	VulkanRenderResource* resource = nullptr;
	while ((resource = deletion_queue.front()) && resource->read_timeline < current_timeline && resource->write_timeline < current_timeline) {
		delete resource;
	}

}

void VulkanRenderResourceManager::ReturnResource(VulkanRenderResource* resource)
{
	std::unique_lock<std::mutex> lock(deletion_queue_mutex);
	deletion_queue.push(resource);
}
