#include "VulkanRenderResourceManager.h"
#include "VulkanRenderContext.h"
#include "VulkanUnitConverter.h"
#include "VulkanRenderCommandQueue.h"
#include "VulkanRenderCommandList.h"
#include "Core/algorithm.h"


void VulkanRenderResourceManager::CreateBuffer_internal(VulkanRenderBufferResource* buffer, const RenderBufferDescriptor& buffer_desc)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	buffer->descriptor = buffer_desc;
	buffer->render_state = buffer_desc.type == RenderBufferType::UPLOAD ? RenderState::COMMON : RenderState::UNINITIALIZED;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = buffer_desc.buffer_size;
	info.usage = VulkanUnitConverter::BufferUsageToVkFlags(buffer_desc.usage);

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VulkanUnitConverter::BufferTypeToVmaUsage(buffer_desc.type);
	alloc_info.flags = VulkanUnitConverter::BufferTypeToVmaFlags(buffer_desc.type);

	vmaCreateBuffer(alloc, &info, &alloc_info, &buffer->buffer, &buffer->alloc, NULL);

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	buffer->read_timeline = timeline;
	buffer->write_timeline = timeline;
}

std::shared_ptr<RenderBufferResource> VulkanRenderResourceManager::CreateBuffer(const RenderBufferDescriptor& buffer_desc)
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource();
	CreateBuffer_internal(new_buffer, buffer_desc);

	return std::shared_ptr<RenderBufferResource>(new_buffer, [](RenderBufferResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnResource(static_cast<VulkanRenderBufferResource*>(resource));
		});
}

void VulkanRenderResourceManager::UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset)
{
	DEFINE_VK_INSTANCE(context);
	auto alloc = context->GetVmaAllocator();
	VulkanRenderBufferResource* buffer = static_cast<VulkanRenderBufferResource*>(resource.get());
	VulkanRenderCommandList* vk_command_list = static_cast<VulkanRenderCommandList*>(list);

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

	vkCmdCopyBuffer(vk_command_list->command_buffer, vk_staging_buffer->buffer, buffer->buffer, 1, &copy);

	VulkanRenderCommandList::VulkanCommandListDependency dep_resource;
	dep_resource.type = VulkanRenderCommandList::VulkanCommandListDependencyType::WRITE;
	dep_resource.current_state = RenderState::COMMON;
	dep_resource.expected_state = RenderState::UNINITIALIZED;

	VulkanRenderCommandList::VulkanCommandListDependency dep_staging;
	dep_staging.type = VulkanRenderCommandList::VulkanCommandListDependencyType::WRITE;
	dep_staging.current_state = RenderState::COMMON;
	dep_staging.expected_state = RenderState::COMMON;

	vk_command_list->AddDependency(resource, dep_resource);
	vk_command_list->AddDependency(staging_buffer, dep_staging);
}

void VulkanRenderResourceManager::ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size)
{
	throw std::runtime_error("ReallocateAndUploadBuffer has been removed, since it violates resource management requirements.\n");
	
	/*DEFINE_VK_INSTANCE(context);
	VulkanRenderBufferResource* buffer = static_cast<VulkanRenderBufferResource*>(resource.get());

	RenderBufferDescriptor new_desc = buffer->descriptor;
	new_desc.buffer_size = size;

	VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource();
	CreateBuffer_internal(new_buffer, new_desc);

	*buffer = std::move(*new_buffer);

	UploadDataToBuffer(list, resource, data, size, 0);*/
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

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	new_texture->read_timeline = timeline;
	new_texture->write_timeline = timeline;

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

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	new_texture->read_timeline = timeline;
	new_texture->write_timeline = timeline;

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

	uint64_t timeline = context->GetCurrentCpuTimelineValue();
	new_texture->read_timeline = timeline;
	new_texture->write_timeline = timeline;

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

	size = RoundUpToPowerOfTwo(size);

	RenderBufferDescriptor buffer_desc(size, RenderBufferType::UPLOAD, RenderBufferUsage::STAGING);

	VulkanRenderBufferResource* new_buffer = new VulkanRenderBufferResource();
	CreateBuffer_internal(new_buffer, buffer_desc);
	
	return std::shared_ptr<RenderBufferResource>(new_buffer, [](RenderBufferResource* resource) {
		static_cast<VulkanRenderResourceManager*>(RenderResourceManager::Get())->ReturnStagingBufferResource(static_cast<VulkanRenderBufferResource*>(resource));
		});
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
	ClearStagingBuffers();
}


void VulkanRenderResourceManager::FlushDeletions()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& alloc = context->GetVmaAllocator();

	std::unique_lock<std::mutex> lock(deletion_queue_mutex);
	std::lock_guard<std::mutex> lock2(staging_buffer_map_mutex);

	uint64_t current_timeline = context->GetCurrentGpuTimelineValue();

	deletion_item resource;
	while (!deletion_queue.empty() && (resource = deletion_queue.front()).resource && resource.resource->read_timeline < current_timeline && resource.resource->write_timeline < current_timeline) {
		if(resource.staging) {
			VulkanRenderBufferResource* buffer = static_cast<VulkanRenderBufferResource*>(resource.resource);

			staging_buffer_map.insert(std::make_pair(buffer->descriptor.buffer_size, buffer));
		}
		else {
			resource.resource->DestroyResource();
			delete resource.resource;
		}
		deletion_queue.pop();
	}

}

void VulkanRenderResourceManager::ReturnResource(VulkanRenderResource* resource)
{
	std::unique_lock<std::mutex> lock(deletion_queue_mutex);
	deletion_queue.push({ resource , false});
}

void VulkanRenderResourceManager::BufferFlushAndMakeAvailable(RenderCommandList* list, std::shared_ptr<RenderBufferResource> buffer, PipelineStage write_scope, PipelineStage read_scope)
{
	VkBufferMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(write_scope);
	barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(read_scope);
	barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
	barrier.buffer = static_cast<VulkanRenderBufferResource*>(buffer.get())->buffer;
	barrier.size = VK_WHOLE_SIZE;

	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.bufferMemoryBarrierCount = 1;
	info.pBufferMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(static_cast<VulkanRenderCommandList*>(list)->command_buffer, &info);

}

void VulkanRenderResourceManager::TransitionImage(RenderCommandList* list, std::shared_ptr<VulkanRenderTextureResource> image, VkImageSubresourceRange range, RenderState source_state, RenderState target_state, PipelineStage source_scope, PipelineStage target_scope)
{

	VkImageMemoryBarrier2 barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(source_scope);
	barrier.dstStageMask = VulkanUnitConverter::PipelineStageToVulkanPipelineStage(target_scope);
	barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
	barrier.image = image->GetImage();
	barrier.subresourceRange = range;


	VkDependencyInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	info.imageMemoryBarrierCount = 1;
	info.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(static_cast<VulkanRenderCommandList*>(list)->command_buffer, &info);
}

void VulkanRenderResourceManager::ReturnStagingBufferResource(VulkanRenderBufferResource* resource)
{
	std::unique_lock<std::mutex> lock(deletion_queue_mutex);
	deletion_queue.push({ resource , true });
}

void VulkanRenderResourceManager::ClearStagingBuffers()
{
	std::lock_guard<std::mutex> lock(staging_buffer_map_mutex);
	for (auto iter : staging_buffer_map) {
		delete iter.second;
	}
	staging_buffer_map.clear();
}

VulkanRenderTexture2DResource* VulkanRenderResourceManager::CreateNonManagedTexture(VkImage image, RenderTexture2DDescriptor desc, RenderState default_state)
{
	DEFINE_VK_INSTANCE(context);
	uint64_t timeline = context->GetCurrentCpuTimelineValue();

	VulkanRenderTexture2DResource* texture = new VulkanRenderTexture2DResource(desc, default_state);

	texture->alloc = VmaAllocation();
	texture->image = image;
	texture->read_timeline = timeline;
	texture->write_timeline = timeline;

	return texture;
}
