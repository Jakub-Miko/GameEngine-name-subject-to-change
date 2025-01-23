#include "VulkanRenderResource.h"
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
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyImage(allocator, image, alloc);
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


void* VulkanRenderTexture2DArrayResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DArrayResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

VulkanRenderTexture2DArrayResource::~VulkanRenderTexture2DArrayResource()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyImage(allocator, image, alloc);
}



void* VulkanRenderTexture2DCubemapResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DCubemapResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

VulkanRenderTexture2DCubemapResource::~VulkanRenderTexture2DCubemapResource()
{
	DEFINE_VK_INSTANCE(context);
	VmaAllocator& allocator = context->GetVmaAllocator();
	vmaDestroyImage(allocator, image, alloc);
}

