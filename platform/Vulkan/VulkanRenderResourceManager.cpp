#include "VulkanRenderResourceManager.h"
#include "VulkanRenderContext.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderCommandList.h"
#include "VulkanRenderResourceStore.h"
#include "Core/algorithm.h"


void VulkanRenderResourceManager::CreateBuffer_internal(VulkanRenderBufferResource* buffer, const RenderBufferDescriptor& buffer_desc, RenderBufferCreationFlags flags)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	buffer->descriptor = buffer_desc;
	buffer->render_state = buffer_desc.type == RenderBufferType::UPLOAD || (bool)(flags & RenderBufferCreationFlags::CREATE_INITIALIZED) 
		? RenderState::COMMON : RenderState::UNINITIALIZED;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = buffer_desc.buffer_size;
	info.usage = VulkanUnitConverter::BufferUsageToVkFlags(buffer_desc.usage);

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VulkanUnitConverter::BufferTypeToVmaUsage(buffer_desc.type);
	alloc_info.flags = VulkanUnitConverter::BufferTypeToVmaFlags(buffer_desc.type);

	auto result = vmaCreateBuffer(alloc, &info, &alloc_info, &buffer->buffer, &buffer->alloc, NULL);

	if(result != VK_SUCCESS) {
		throw std::runtime_error("Vulkan buffer creation failed.\n");
	}

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	buffer->read_timeline = timeline;
	buffer->write_timeline = timeline;
}

std::shared_ptr<RenderBufferResource> VulkanRenderResourceManager::CreateBuffer(const RenderBufferDescriptor& buffer_desc, RenderBufferCreationFlags flags)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource();
	CreateBuffer_internal(new_buffer, buffer_desc, flags);

	return std::shared_ptr<RenderBufferResource>(new_buffer, [](RenderBufferResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderBufferResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToBuffer(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset)
{
	DEFINE_VK_INSTANCE(context);
	auto alloc = context->GetVmaAllocator();
	VulkanRenderBufferResource* buffer = static_cast<VulkanRenderBufferResource*>(resource.get());
	auto vk_command_list = std::static_pointer_cast<VulkanRenderCommandList>(list);
	vk_command_list->OutsideRenderPass();


	if (buffer->descriptor.buffer_size < size + offset) {
		throw std::runtime_error("Attempted to upload data to a buffer of invalid size, check the size and offset of the data.\n");
	}

	// For now handle upload resources by using staging buffers, since we need asynchronous uploads
	//if (buffer->descriptor.type == RenderBufferType::UPLOAD) {
	//	throw std::runtime_error("Unexpected behaviour might occur when asynchronously uploading to upload resources\n");
	//	vmaCopyMemoryToAllocation(alloc, data, buffer->alloc, offset, size);
	//	return;
	//}

	auto staging_buffer = GetStagingBuffer(size);

	VulkanRenderBufferResource* vk_staging_buffer = static_cast<VulkanRenderBufferResource*>(staging_buffer.get());

	vmaCopyMemoryToAllocation(alloc, data, vk_staging_buffer->alloc, 0, size);

	VkBufferCopy copy = {};
	copy.dstOffset = offset;
	copy.size = size;
	copy.srcOffset = 0;

	vk_command_list->AddDependency(resource, VulkanCommandListDependencyType::WRITE, RenderState::COMMON);
	vk_command_list->AddDependency(staging_buffer, VulkanCommandListDependencyType::WRITE, RenderState::COMMON);
	vkCmdCopyBuffer(vk_command_list->command_buffer, vk_staging_buffer->buffer, buffer->buffer, 1, &copy);


}

void VulkanRenderResourceManager::ReallocateAndUploadBuffer(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size)
{\
	throw std::runtime_error("ReallocateAndUploadBuffer has been removed, since it violates resource management requirements.\n");
	
	// DEFINE_VK_INSTANCE(context);
	// VulkanRenderBufferResource* buffer = static_cast<VulkanRenderBufferResource*>(resource.get());

	// RenderBufferDescriptor new_desc = buffer->descriptor;
	// new_desc.buffer_size = size;

	// VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource(); 
	// CreateBuffer_internal(new_buffer, new_desc, RenderBufferCreationFlags::NONE);

	// UploadDataToBuffer(list, resource, data, size, 0);
}

void VulkanRenderResourceManager::CopyBufferData(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderBufferResource> source, std::shared_ptr<RenderBufferResource> destination, size_t source_offset, size_t source_size, size_t destination_offset)
{
	auto vk_command_list = std::static_pointer_cast<VulkanRenderCommandList>(list);
	vk_command_list->OutsideRenderPass();

	VkBufferCopy copy = {};
	copy.dstOffset = destination_offset;
	copy.size = source_size;
	copy.srcOffset = source_offset;

	vk_command_list->AddDependency(source, VulkanCommandListDependencyType::READ, RenderState::COMMON);
	vk_command_list->AddDependency(destination, VulkanCommandListDependencyType::WRITE, RenderState::COMMON);

	VulkanRenderBufferResource* source_buffer = static_cast<VulkanRenderBufferResource*>(source.get());
	VulkanRenderBufferResource* destination_buffer = static_cast<VulkanRenderBufferResource*>(destination.get());


	vkCmdCopyBuffer(vk_command_list->command_buffer, source_buffer->buffer, destination_buffer->buffer, 1, &copy);
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
	image_info.mipLevels = buffer_desc.mipmap_levels; 
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(usage);
	

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VkImageView view;
	VmaAllocation allocation;

	vmaCreateImage(alloc,&image_info,&alloc_info,&image,&allocation,NULL);
	
	VkImageSubresourceRange range = {};
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = VK_REMAINING_ARRAY_LAYERS;
	range.levelCount = VK_REMAINING_MIP_LEVELS;
	range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(buffer_desc.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	view_info.format = image_info.format;
	view_info.image = image;
	view_info.subresourceRange = range;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkCreateImageView(context->GetVkDevice(), &view_info, NULL, &view);


	VulkanRenderTexture2DResource* new_texture = new VulkanRenderTexture2DResource(buffer_desc, RenderState::UNINITIALIZED);

	new_texture->alloc = allocation;
	new_texture->image = image;
	new_texture->default_state = default_state;
	new_texture->views.push_back(view);

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	new_texture->read_timeline = timeline;
	new_texture->write_timeline = timeline;

	return std::shared_ptr<RenderTexture2DResource>(new_texture, [](RenderTexture2DResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderTexture2DResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToTexture2D(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
	DEFINE_VK_INSTANCE(context);
	auto alloc = context->GetVmaAllocator();
	VulkanRenderTextureResource* texture = static_cast<VulkanRenderTextureResource*>(resource->GetExtensionData());
	auto vk_command_list = std::static_pointer_cast<VulkanRenderCommandList>(list);
	vk_command_list->OutsideRenderPass();

	auto res = texture->GetResolution();
	auto size = VulkanUnitConverter::TextureFormatToTexelSize(texture->GetFormat()) * res.x * res.y;

	if (res.x < width + offset_x || res.y < height + offset_y) {
		throw std::runtime_error("Attempted to upload data to a buffer of invalid size, check the size and offset of the data.\n");
	}

	auto staging_buffer = GetStagingBuffer(size);

	VulkanRenderBufferResource* vk_staging_buffer = static_cast<VulkanRenderBufferResource*>(staging_buffer.get());

	vmaCopyMemoryToAllocation(alloc, data, vk_staging_buffer->alloc, 0, size);

	VkImageSubresourceLayers layers = {};
	layers.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(texture->GetFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	layers.baseArrayLayer = 0;
	layers.mipLevel = level;
	layers.layerCount = 1;

	VkBufferImageCopy copy = {};
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageExtent = { res.x, res.y, 1 };
	copy.imageOffset = { 0,0,0 };
	copy.imageSubresource = layers;

	vk_command_list->AddDependency(resource, VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST);
	vk_command_list->AddDependency(staging_buffer, VulkanCommandListDependencyType::WRITE, RenderState::COMMON);
	vkCmdCopyBufferToImage(*vk_command_list->GetVkCommandBuffer(), vk_staging_buffer->buffer, texture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

void VulkanRenderResourceManager::GenerateMIPs(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DResource> resource)
{
}

void VulkanRenderResourceManager::UploadToTexture2DFromFile(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level)
{
}

std::shared_ptr<RenderTexture2DResource> VulkanRenderResourceManager::CreateTextureFromFile(std::shared_ptr<RenderCommandList>  list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler)
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
	image_info.mipLevels = buffer_desc.mipmap_levels; 
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(usage);


	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VkImageView view;
	VmaAllocation allocation;

	vmaCreateImage(alloc, &image_info, &alloc_info, &image, &allocation, NULL);

	VkImageSubresourceRange range = {};
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = VK_REMAINING_ARRAY_LAYERS;
	range.levelCount = VK_REMAINING_MIP_LEVELS;
	range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(buffer_desc.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	view_info.format = image_info.format;
	view_info.image = image;
	view_info.subresourceRange = range;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	vkCreateImageView(context->GetVkDevice(), &view_info, NULL, &view);

	VulkanRenderTexture2DArrayResource* new_texture = new VulkanRenderTexture2DArrayResource(buffer_desc, RenderState::UNINITIALIZED);

	new_texture->alloc = allocation;
	new_texture->image = image;
	new_texture->default_state = default_state;
	new_texture->views.push_back(view);

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	new_texture->read_timeline = timeline;
	new_texture->write_timeline = timeline;

	return std::shared_ptr<RenderTexture2DArrayResource>(new_texture, [](RenderTexture2DArrayResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderTexture2DArrayResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToTexture2DArray(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
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
	image_info.mipLevels = buffer_desc.mipmap_levels; 
	image_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VulkanUnitConverter::TextureUsageToVkTextureUsage(usage);


	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = NULL;

	VkImage image;
	VkImageView view;
	VmaAllocation allocation;

	vmaCreateImage(alloc, &image_info, &alloc_info, &image, &allocation, NULL);

	VkImageSubresourceRange range = {};
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = VK_REMAINING_ARRAY_LAYERS;
	range.levelCount = VK_REMAINING_MIP_LEVELS;
	range.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(buffer_desc.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	view_info.format = image_info.format;
	view_info.image = image;
	view_info.subresourceRange = range;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

	vkCreateImageView(context->GetVkDevice(), &view_info, NULL, &view);

	VulkanRenderTexture2DCubemapResource* new_texture = new VulkanRenderTexture2DCubemapResource(buffer_desc, RenderState::UNINITIALIZED);

	new_texture->alloc = allocation;
	new_texture->image = image;
	new_texture->default_state = default_state;
	new_texture->views.push_back(view);

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	new_texture->read_timeline = timeline;
	new_texture->write_timeline = timeline;

	return std::shared_ptr<RenderTexture2DCubemapResource>(new_texture, [](RenderTexture2DCubemapResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderTexture2DCubemapResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToTexture2DCubemap(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
}

std::shared_ptr<RenderFrameBufferResource> VulkanRenderResourceManager::CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc)
{
	return std::shared_ptr<RenderFrameBufferResource>(new VulkanRenderFrameBufferResource(buffer_desc));
}

void VulkanRenderResourceManager::CreateConstantBufferDescriptor(const VulkanRenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource)
{
	DEFINE_VK_INSTANCE(context);

	auto vk_desc_table = std::static_pointer_cast<VulkanRenderDescriptorAllocation>(table);
	auto vk_buffer = std::static_pointer_cast<VulkanRenderBufferResource>(resource);

	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = vk_buffer->GetBuffer();
	buffer_info.offset = 0;
	buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet write_desc = {};
	write_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_desc.descriptorCount = 1;
	write_desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_desc.dstBinding = index;
	write_desc.dstSet = vk_desc_table->descritor_set;
	write_desc.pBufferInfo = &buffer_info;

	vkUpdateDescriptorSets(context->GetVkDevice(), 1, &write_desc ,0,NULL);
}

void VulkanRenderResourceManager::CreateTexture2DDescriptor(const VulkanRenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource)
{
	DEFINE_VK_INSTANCE(context);

	auto vk_desc_table = std::static_pointer_cast<VulkanRenderDescriptorAllocation>(table);
	auto vk_texture = std::static_pointer_cast<VulkanRenderTexture2DResource>(resource);

	VkDescriptorImageInfo image_info = {};
	image_info.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	image_info.imageView = vk_texture->GetImageView();
	image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(vk_texture->GetSampler())->GetSampler();

	VkWriteDescriptorSet write_desc = {};
	write_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_desc.descriptorCount = 1;
	write_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_desc.dstBinding = index;
	write_desc.dstSet = vk_desc_table->descritor_set;
	write_desc.pImageInfo = &image_info;

	vkUpdateDescriptorSets(context->GetVkDevice(), 1, &write_desc ,0,NULL);
}

std::shared_ptr<Awaitable<read_pixel_data>> VulkanRenderResourceManager::GetPixelValue(std::shared_ptr<RenderFrameBufferResource> framebuffer, int color_attachment_index, float x, float y)
{
	DEFINE_VK_INSTANCE(context);
	auto list = std::static_pointer_cast<VulkanRenderCommandList>(Renderer::Get()->GetRenderCommandList());
	auto vk_command_buffer = *list->GetVkCommandBuffer();
	auto queue = static_cast<VulkanRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	auto attachment = framebuffer->GetBufferDescriptor().color_attachments[color_attachment_index].resource;
	if(attachment->GetResourceType() != RenderResourceType::RenderTexture2DResource) {
		throw std::runtime_error("GetPixelValue only supports texture 2D attachments.\n");
	}
	auto texture = std::static_pointer_cast<VulkanRenderTexture2DResource>(attachment);
	auto desc = texture->GetBufferDescriptor();

	if(x >= desc.width || x < 0 || y >= desc.height || y < 0) {
		throw std::runtime_error("Invalid x and y coordinates passed to GetPixelValue.\n");
	}

	auto size = VulkanUnitConverter::TextureFormatToTexelSize(desc.format);
	auto staging_buf = std::static_pointer_cast<VulkanRenderBufferResource>(GetStagingBuffer(size));

	VkBufferImageCopy copy_info = {};
	copy_info.bufferOffset = 0;
	copy_info.bufferRowLength = 0;
	copy_info.bufferImageHeight = 0;
	copy_info.imageOffset = { (int)(x * desc.width), (int)(y * desc.height), 0};
	copy_info.imageExtent = { 1, 1, 1};
	copy_info.imageSubresource.aspectMask = VulkanUnitConverter::IsTextureFormatDepth(desc.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	copy_info.imageSubresource.baseArrayLayer = 0;
	copy_info.imageSubresource.layerCount = 1;
	copy_info.imageSubresource.mipLevel = 0;

	list->AddDependency(staging_buf, VulkanCommandListDependencyType::WRITE, RenderState::COMMON);
	list->AddDependency(texture, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_TRANSFER_SRC);

	vkCmdCopyImageToBuffer(vk_command_buffer, texture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_buf->GetBuffer(), 1, &copy_info);
	
	queue->ExecuteRenderCommandList(list);
	auto signaled = context->GetCurrentCpuTimelineValue();
	
	auto is_available_func = [signaled, context]() -> bool {
		auto gpu_value = context->GetCurrentGpuTimelineValue();
		return gpu_value >= signaled;
	};

	auto wait_func = [queue, signaled]() {
		queue->WaitForValue(signaled);
	};

	auto format = desc.format;

	auto get_value_func = [queue, signaled, context, staging_buf, format]() -> read_pixel_data {
		queue->WaitForValue(signaled);
		switch (format)
		{
		case TextureFormat::RGBA_32FLOAT:
		{
			glm::vec4 value; //This is just fucked, what am i even doing, i just realized this is a suicide attempt. (2 years later and I realize i should have maybe described why it is fucked because i have no idea anymore.)
			vmaCopyAllocationToMemory(context->GetVmaAllocator(), staging_buf->alloc, 0,glm::value_ptr(value), sizeof(value));
			return read_pixel_data(value);
			break;
		}
		case TextureFormat::RGB_32FLOAT:
		{
			glm::vec3 value; //This is just fucked, what am i even doing, i just realized this is a suicide attempt. (2 years later and I realize i should have maybe described why it is fucked because i have no idea anymore.)
			vmaCopyAllocationToMemory(context->GetVmaAllocator(), staging_buf->alloc, 0,glm::value_ptr(value), sizeof(value));
			return read_pixel_data(value);
			break;
		}
		case TextureFormat::R_UNSIGNED_INT:
		{
			unsigned int value; //This is just fucked, what am i even doing, i just realized this is a suicide attempt. (2 years later and I realize i should have maybe described why it is fucked because i have no idea anymore.)
			vmaCopyAllocationToMemory(context->GetVmaAllocator(), staging_buf->alloc, 0,&value, sizeof(value));
			return read_pixel_data(value);
			break;
		}
		default:
			throw std::runtime_error("GetPixelValue currently doesn't support this data type");
		}
	};


	auto awaitable = std::make_shared<CustomAwaitable<read_pixel_data>>(wait_func, is_available_func, get_value_func);


	return awaitable;
}

void VulkanRenderResourceManager::CreateTexture2DArrayDescriptor(const VulkanRenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource)
{
	DEFINE_VK_INSTANCE(context);

	auto vk_desc_table = std::static_pointer_cast<VulkanRenderDescriptorAllocation>(table);
	auto vk_texture = std::static_pointer_cast<VulkanRenderTexture2DArrayResource>(resource);

	VkDescriptorImageInfo image_info = {};
	image_info.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	image_info.imageView = vk_texture->GetImageView();
	image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(vk_texture->GetSampler())->GetSampler();

	VkWriteDescriptorSet write_desc = {};
	write_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_desc.descriptorCount = 1;
	write_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_desc.dstBinding = index;
	write_desc.dstSet = vk_desc_table->descritor_set;
	write_desc.pImageInfo = &image_info;

	vkUpdateDescriptorSets(context->GetVkDevice(), 1, &write_desc ,0,NULL);
}

void VulkanRenderResourceManager::CreateTexture2DCubemapDescriptor(const VulkanRenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource)
{
	DEFINE_VK_INSTANCE(context);

	auto vk_desc_table = std::static_pointer_cast<VulkanRenderDescriptorAllocation>(table);
	auto vk_texture = std::static_pointer_cast<VulkanRenderTexture2DCubemapResource>(resource);

	VkDescriptorImageInfo image_info = {};
	image_info.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	image_info.imageView = vk_texture->GetImageView();
	image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(vk_texture->GetSampler())->GetSampler();

	VkWriteDescriptorSet write_desc = {};
	write_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_desc.descriptorCount = 1;
	write_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_desc.dstBinding = index;
	write_desc.dstSet = vk_desc_table->descritor_set;
	write_desc.pImageInfo = &image_info;

	vkUpdateDescriptorSets(context->GetVkDevice(), 1, &write_desc ,0,NULL);
}

void VulkanRenderResourceManager::CopyFrameBufferDepthAttachment(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer)
{
	DEFINE_VK_INSTANCE(context);
	VulkanRenderTextureResource* source_depth = static_cast<VulkanRenderTextureResource*>(
		source_frame_buffer->GetBufferDescriptor().depth_stencil_attachment.resource->GetExtensionData());

	VulkanRenderTextureResource* destination_depth = static_cast<VulkanRenderTextureResource*>(
		destination_frame_buffer->GetBufferDescriptor().depth_stencil_attachment.resource->GetExtensionData());

	if(destination_depth->GetResolution() != source_depth->GetResolution()) {
		throw std::runtime_error("Resolution of depth attachments must match to allow depth copying.\n");
	}

	auto res = destination_depth->GetResolution();

	auto vk_command_list = std::static_pointer_cast<VulkanRenderCommandList>(list);
	vk_command_list->OutsideRenderPass();

	vk_command_list->AddDependency(source_frame_buffer->GetBufferDescriptor().depth_stencil_attachment.resource, VulkanCommandListDependencyType::READ, RenderState::TEXTURE_TRANSFER_SRC);
	vk_command_list->AddDependency(destination_frame_buffer->GetBufferDescriptor().depth_stencil_attachment.resource, VulkanCommandListDependencyType::WRITE, RenderState::TEXTURE_TRANSFER_DST);

	VkImageSubresourceLayers layers = {};
	layers.mipLevel = 0;
	layers.baseArrayLayer = 0;
	layers.layerCount = 1;
	layers.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageCopy region = {};
	region.srcSubresource = layers;
	region.dstSubresource = layers;
	region.srcOffset = {0,0};
	region.dstOffset = {0,0};
	region.extent = {res.x, res.y, 1};

	vkCmdCopyImage(*vk_command_list->GetVkCommandBuffer(), source_depth->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	 destination_depth->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanRenderResourceManager::SetFrameBufferColorAttachment(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderFrameBufferResource> framebuffer, std::shared_ptr<RenderResource> new_attachment, int index, int level)
{
	auto vk_framebuffer = std::static_pointer_cast<VulkanRenderFrameBufferResource>(framebuffer);
	auto& desc = GetAdjustableFrameBufferDescriptor(framebuffer);

	if(desc.color_attachments.size() <= index) {
		throw std::runtime_error("Invalid attachment index.\n");
	}

	desc.color_attachments[index].level = level;
	desc.color_attachments[index].resource = new_attachment;

	vk_framebuffer->dirty = true;
}

int VulkanRenderResourceManager::GetCubemapFaceIndex(RenderCubemapFace face)
{
    int indicies[6] = {};
	indicies[(int)RenderCubemapFace::CUBEMAP_RIGHT] = 0;
	indicies[(int)RenderCubemapFace::CUBEMAP_LEFT] = 1;
	indicies[(int)RenderCubemapFace::CUBEMAP_TOP] = 3; // Y coordinate is flipped in vulkan
	indicies[(int)RenderCubemapFace::CUBEMAP_BOTTOM] = 2; // Y coordinate is flipped in vulkan
	indicies[(int)RenderCubemapFace::CUBEMAP_FRONT] = 4;
	indicies[(int)RenderCubemapFace::CUBEMAP_BACK] = 5;
	return indicies[(int)face];
}

std::shared_ptr<RenderBufferResource> VulkanRenderResourceManager::GetStagingBuffer(size_t size)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();
	std::unique_lock<std::mutex> lock(staging_buffer_map_mutex);

	auto fnd = staging_buffer_map.lower_bound(size);
	if (fnd != staging_buffer_map.end()) {
		auto temp = fnd->second;
		staging_buffer_map.erase(fnd);
		return std::shared_ptr<RenderBufferResource>(temp, [](RenderBufferResource* resource) {
			static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnStagingBufferResource(static_cast<VulkanRenderBufferResource*>(resource));
			});
	}
	lock.unlock();

	size = std::max((size_t)256, RoundUpToPowerOfTwo(size));

	RenderBufferDescriptor buffer_desc(size, RenderBufferType::UPLOAD, RenderBufferUsage::STAGING);

	VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource();
	CreateBuffer_internal(new_buffer, buffer_desc, RenderBufferCreationFlags::NONE);
	
	return std::shared_ptr<RenderBufferResource>(new_buffer, [](RenderBufferResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnStagingBufferResource(static_cast<VulkanRenderBufferResource*>(resource));
		});
}

void VulkanRenderResourceManager::AddToDeferredDestructionQueue(VulkanDeferredDestruction *resource, uint32_t last_usage_timeline_value)
{
	std::lock_guard<std::recursive_mutex> lock(deletion_queue_mutex);
	deletion_item item = {};
	item.deferred_destroy_resource = resource;
	item.type = deletion_item_type::DEFERRED_DESTROY_RESOURCE;
	item.deletion_timeline = last_usage_timeline_value;
	deletion_queue.push(item);
}

void VulkanRenderResourceManager::Update()
{
	FlushDeletions();

}

VulkanRenderResourceManager::VulkanRenderResourceManager() : deletion_queue(), deletion_queue_mutex(), staging_buffer_map() , staging_buffer_map_mutex()
{
}

VulkanRenderResourceManager::~VulkanRenderResourceManager()
{
	DEFINE_VK_INSTANCE(context);
	vkDeviceWaitIdle(context->GetVkbDevice());
	FlushDeletions(true);
	ClearStagingBuffers();
}


void VulkanRenderResourceManager::FlushDeletions(bool force)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	std::unique_lock<std::recursive_mutex> lock(deletion_queue_mutex);
	std::lock_guard<std::mutex> lock2(staging_buffer_map_mutex);

	uint64_t current_timeline = context->GetCurrentGpuTimelineValue();

	deletion_item resource;
	while (!deletion_queue.empty() && (resource = deletion_queue.front()).resource && (resource.deletion_timeline < current_timeline || force)) {
		switch (resource.type)
		{
		case deletion_item_type::RESOURCE:
			resource.resource->DestroyResource();
			delete resource.resource;
			break;
		case deletion_item_type::STAGING_BUFFER:
		{
			VulkanRenderBufferResource* buffer = static_cast<VulkanRenderBufferResource*>(resource.resource);
			staging_buffer_map.insert(std::make_pair(buffer->descriptor.buffer_size, buffer));
			break;
		}
		case deletion_item_type::DEFERRED_DESTROY_RESOURCE:
		{
			if(resource.deferred_destroy_resource->Destroy()) {
				delete resource.deferred_destroy_resource;
			}
		}
		break;
		default:
			throw std::runtime_error("Invalid deletion type.\n");
		}
		
		deletion_queue.pop();
	}

}

void VulkanRenderResourceManager::ReturnResource(VulkanRenderResource* resource)
{
	std::lock_guard<std::recursive_mutex> lock(deletion_queue_mutex);
	deletion_item item;
	item.resource = resource;
	item.type = deletion_item_type::RESOURCE;
	item.deletion_timeline = std::max(resource->read_timeline, resource->write_timeline);
	deletion_queue.push(item);
}

void VulkanRenderResourceManager::BufferBarrier(RenderCommandList* list, std::shared_ptr<RenderBufferResource> buffer,
	PipelineStage write_scope, PipelineStage read_scope, 
	VulkanCommandListDependencyType src_access , VulkanCommandListDependencyType dst_access)
{
	VkBufferMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(write_scope);
	barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(read_scope);
	barrier.srcAccessMask = VulkanUnitConverter::DependencyToVkAccess(src_access);
	barrier.dstAccessMask = VulkanUnitConverter::DependencyToVkAccess(dst_access);
	barrier.buffer = static_cast<VulkanRenderBufferResource*>(buffer.get())->buffer;
	barrier.size = VK_WHOLE_SIZE;

	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.bufferMemoryBarrierCount = 1;
	info.pBufferMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(static_cast<VulkanRenderCommandList*>(list)->command_buffer, &info);

}

std::shared_ptr<RenderResourceStore> VulkanRenderResourceManager::CreateResourceStore( const RenderResourceStoreDescriptor& store_descriptor) {
	DEFINE_VK_INSTANCE(context);
	auto* store = new VulkanRenderResourceStore(store_descriptor);

	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = 0;
	layout_binding.descriptorType = VulkanUnitConverter::RootParameterTypeToDescritorType(store_descriptor.resource_parameter_type);
	layout_binding.descriptorCount = store_descriptor.max_resource_count;
	layout_binding.stageFlags = VK_SHADER_STAGE_ALL;

	VkDescriptorBindingFlags binding_flags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
		| VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		| VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo layout_binding_flags = {};
	layout_binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	layout_binding_flags.bindingCount = 1;
	layout_binding_flags.pBindingFlags = &binding_flags;

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &layout_binding;
	layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	layout_info.pNext = &layout_binding_flags;

	vkCreateDescriptorSetLayout(context->GetVkDevice(), &layout_info, nullptr, &store->descriptor_set_layout);

	VkDescriptorPoolSize pool_size = {};
	pool_size.descriptorCount = store_descriptor.max_resource_count;
	pool_size.type = VulkanUnitConverter::RootParameterTypeToDescritorType(store_descriptor.resource_parameter_type);

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.maxSets = 1;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	vkCreateDescriptorPool(context->GetVkDevice(), &pool_info, nullptr, &store->descriptor_pool);

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = store->descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &store->descriptor_set_layout;

	vkAllocateDescriptorSets(context->GetVkDevice(), &alloc_info, &store->descriptor_set);

	return std::shared_ptr<VulkanRenderResourceStore>(store, [this](VulkanRenderResourceStore* resource) {
		AddToDeferredDestructionQueue(resource, std::max(resource->read_timeline, resource->write_timeline));
	});
}

void VulkanRenderResourceManager::TransitionImage(RenderCommandList*  list, VulkanRenderTextureResource* image, VkImageSubresourceRange range, RenderState source_state, RenderState target_state,
                                                  PipelineStage source_scope, PipelineStage target_scope,
                                                  VulkanCommandListDependencyType src_access, VulkanCommandListDependencyType dst_access)
{
	VkImageMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(source_scope);
	barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(target_scope);
	barrier.srcAccessMask = VulkanUnitConverter::DependencyToVkAccess(src_access);
	barrier.dstAccessMask = VulkanUnitConverter::DependencyToVkAccess(dst_access);
	barrier.image = image->GetImage();
	barrier.subresourceRange = range;
	barrier.oldLayout = VulkanUnitConverter::RenderStateToTextureLayout(source_state);
	barrier.newLayout = VulkanUnitConverter::RenderStateToTextureLayout(target_state);

	if (source_state != target_state) {
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}

	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.imageMemoryBarrierCount = 1;
	info.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(static_cast<VulkanRenderCommandList*>(list)->command_buffer, &info);
}

void VulkanRenderResourceManager::ReturnStagingBufferResource(VulkanRenderBufferResource* resource)
{
	std::lock_guard<std::recursive_mutex> lock(deletion_queue_mutex);
	deletion_item item;
	item.resource = resource;
	item.type = deletion_item_type::STAGING_BUFFER;
	item.deletion_timeline = std::max(resource->read_timeline, resource->write_timeline);
	deletion_queue.push(item);
}

void VulkanRenderResourceManager::ClearStagingBuffers()
{
	std::lock_guard<std::mutex> lock(staging_buffer_map_mutex);
	for (auto iter : staging_buffer_map) {
		static_cast<VulkanRenderBufferResource*>(iter.second)->DestroyResource();
		delete iter.second;
	}
	staging_buffer_map.clear();
}

VulkanRenderTexture2DResource* VulkanRenderResourceManager::CreateNonManagedTexture(VkImage image, VkImageView view, RenderTexture2DDescriptor desc, RenderState default_state, RenderState initial_state)
{
	DEFINE_VK_INSTANCE(context);
	uint64_t timeline = context->GetCurrentCpuTimelineValue();

	VulkanRenderTexture2DResource* texture = new VulkanRenderTexture2DResource(desc, initial_state);

	texture->alloc = VmaAllocation();
	texture->image = image;
	texture->read_timeline = timeline;
	texture->write_timeline = timeline;
	texture->default_state = default_state;
	texture->views.push_back(view);

	return texture;
}
