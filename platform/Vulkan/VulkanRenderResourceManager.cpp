#include "VulkanRenderResourceManager.h"

std::shared_ptr<RenderBufferResource> VulkanRenderResourceManager::CreateBuffer(const RenderBufferDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderBufferResource>();
}

void VulkanRenderResourceManager::UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset)
{
}

void VulkanRenderResourceManager::ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size)
{
}

std::shared_ptr<RenderTexture2DResource> VulkanRenderResourceManager::CreateTexture(const RenderTexture2DDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderTexture2DResource>();
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

VulkanRenderResourceManager::VulkanRenderResourceManager()
{
}

VulkanRenderResourceManager::~VulkanRenderResourceManager()
{
}

void VulkanRenderResourceManager::ReturnBufferResource(RenderBufferResource* resource)
{
}

void VulkanRenderResourceManager::ReturnTexture2DResource(RenderTexture2DResource* resource)
{
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
