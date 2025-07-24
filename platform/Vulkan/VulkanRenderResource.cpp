#include "VulkanRenderResource.h"
#include "VulkanRenderResourceManager.h"
#include "VulkanUnitConverter.h"
#include "vulkan/vulkan.h"
#include <stdexcept>

void* VulkanRenderBufferResource::Map()
{
	DEFINE_VK_INSTANCE(context);
	void* data;
	vmaMapMemory(context->GetVmaAllocator(), alloc, &data);
	return data;
}

void VulkanRenderBufferResource::UnMap()
{
	DEFINE_VK_INSTANCE(context);
	vmaUnmapMemory(context->GetVmaAllocator(), alloc);
}

VulkanRenderBufferResource::~VulkanRenderBufferResource()
{

}

void VulkanRenderBufferResource::DestroyResource()
{
	if (write_timeline == -1)
		return;

	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyBuffer(allocator, buffer, alloc);
}



void* VulkanRenderTexture2DResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

VulkanRenderTexture2DResource::~VulkanRenderTexture2DResource()
{

}

void VulkanRenderTexture2DResource::DestroyResource()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyImage(allocator, image, alloc);
	vkDestroyImageView(context->GetVkDevice(), view, NULL);
}



VulkanTextureSampler::~VulkanTextureSampler()
{
	DEFINE_VK_INSTANCE(context);
	vkDestroySampler(context->GetVkDevice(), sampler, NULL);
}

VulkanTextureSampler::VulkanTextureSampler(const TextureSamplerDescritor& desc) : TextureSampler(desc), sampler()
{
	VkSamplerCustomBorderColorCreateInfoEXT border_color = {};
	border_color.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
	border_color.format = VK_FORMAT_UNDEFINED;
	border_color.customBorderColor = VkClearColorValue{ *glm::value_ptr(desc.border_color) };

	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.addressModeU = VulkanUnitConverter::TextureAddressModeToVulkanAddressMode(desc.AddressMode_U);
	info.addressModeV = VulkanUnitConverter::TextureAddressModeToVulkanAddressMode(desc.AddressMode_V);
	info.addressModeW = VulkanUnitConverter::TextureAddressModeToVulkanAddressMode(desc.AddressMode_W);
	info.anisotropyEnable = desc.enable_anisotropy;
	info.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
	info.compareEnable = desc.comparison_mode != DepthComparisonMode::DISABLED;
	info.compareOp = VulkanUnitConverter::DepthComparisonModeToVulkanCompareFunc(desc.comparison_mode);
	info.magFilter = VulkanUnitConverter::TextureFilterToMinMagFilter(desc.filter);
	info.minFilter = VulkanUnitConverter::TextureFilterToMinMagFilter(desc.filter);
	info.maxAnisotropy = 1.0f;
	info.maxLod = desc.max_LOD;
	info.minLod = desc.min_LOD;
	info.mipLodBias = desc.LOD_bias;
	info.mipmapMode = VulkanUnitConverter::TextureFilterToMipFilter(desc.filter);
	info.unnormalizedCoordinates = false;
	info.pNext = &border_color;
	
	DEFINE_VK_INSTANCE(context);
	vkCreateSampler(context->GetVkDevice(), &info, NULL, &sampler);

}

void* VulkanRenderFrameBufferResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderFrameBufferResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

VulkanRenderFrameBufferResource::VulkanRenderFrameBufferResource(const RenderFrameBufferDescriptor& desc, RenderState initial_state, unsigned int render_id) 
	: RenderFrameBufferResource(desc, initial_state), attachment_info_store(), rendering_info(), dirty(true)
{
	RecalculateRenderingInfo();
}

void VulkanRenderFrameBufferResource::RecalculateRenderingInfo()
{
	bool has_color = descriptor.color_attachments.size() != 0;
	bool has_depth = descriptor.depth_stencil_attachment.resource != nullptr;

	if (!has_depth && !has_color) {
		throw std::runtime_error("Framebuffer needs to have at least either a color or a depth attachment, it can't be completely empty.\n");
	}

	auto default_resource = has_color ? descriptor.color_attachments[0].resource : descriptor.depth_stencil_attachment.resource;
	auto default_attachment = static_cast<VulkanRenderTextureResource*>(default_resource->GetExtensionData());


	VkRect2D extent;
	extent.offset = { 0 , 0 };
	extent.extent = { default_attachment->GetResolution().x , default_attachment->GetResolution().y };

	VkRenderingInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	info.renderArea = extent;
	info.layerCount = default_attachment->GetArrayLayerCount();
	info.viewMask = 0;
	info.colorAttachmentCount = descriptor.color_attachments.size();

	VkRenderingAttachmentInfo attachment_info = {};
	attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;

	attachment_info_store.reserve(info.colorAttachmentCount + 1);
	
	if (has_depth) {
		auto depth_texture = static_cast<VulkanRenderTextureResource*>(descriptor.depth_stencil_attachment.resource->GetExtensionData());
		attachment_info.imageView = depth_texture->GetImageView();
		attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		attachment_info_store.push_back(attachment_info);
		info.pDepthAttachment = &attachment_info_store[0];
	}
	else {
		attachment_info_store.push_back(VkRenderingAttachmentInfo());
	}

	attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	if (has_color) {
		for (auto color_attachment : descriptor.color_attachments) {
			VkRenderingAttachmentInfo color_attachment_info = {};
			auto texture = static_cast<VulkanRenderTexture2DResource*>(color_attachment.resource->GetExtensionData());
			attachment_info.imageView = texture->GetImageView();
			attachment_info_store.push_back(attachment_info);
		}
		info.pColorAttachments = &attachment_info_store[1];
	}

	rendering_info = info;
}

void* VulkanRenderTexture2DArrayResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DArrayResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DArrayResource::DestroyResource()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyImage(allocator, image, alloc);
	vkDestroyImageView(context->GetVkDevice(), view, NULL);
}

VulkanRenderTexture2DArrayResource::~VulkanRenderTexture2DArrayResource()
{

}



void* VulkanRenderTexture2DCubemapResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DCubemapResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DCubemapResource::DestroyResource()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyImage(allocator, image, alloc);
	vkDestroyImageView(context->GetVkDevice(), view, NULL);
}

VulkanRenderTexture2DCubemapResource::~VulkanRenderTexture2DCubemapResource()
{
}

