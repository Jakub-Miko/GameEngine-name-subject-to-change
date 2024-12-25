#include "VulkanRenderResource.h"
#include <stdexcept>

void* VulkanRenderBufferResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderBufferResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}



void* VulkanRenderTexture2DResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}



VulkanTextureSampler::VulkanTextureSampler(const TextureSamplerDescritor& desc) : TextureSampler(desc)
{

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



void* VulkanRenderTexture2DCubemapResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void VulkanRenderTexture2DCubemapResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

