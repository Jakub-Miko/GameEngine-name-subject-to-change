#include "RenderResource.h"
#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderResource.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderResource.h>
#endif

std::shared_ptr<TextureSampler> TextureSampler::CreateSampler(const TextureSamplerDescritor& desc)
{
#ifdef OpenGL_API
	return std::shared_ptr<OpenGLTextureSampler>(new OpenGLTextureSampler(desc));
#elif defined Vulkan_API
	return std::shared_ptr<VulkanTextureSampler>(new VulkanTextureSampler(desc));
#endif
}

std::shared_ptr<RenderTexture2DResource> RenderFrameBufferDescriptor::GetColorAttachmentAsTexture(int index) const
{
	if (color_attachments[index].resource->GetResourceType() == RenderResourceType::RenderTexture2DResource) {
		return std::static_pointer_cast<RenderTexture2DResource>(color_attachments[index].resource);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DArrayResource> RenderFrameBufferDescriptor::GetColorAttachmentAsTextureArray(int index) const
{
	if (color_attachments[index].resource->GetResourceType() == RenderResourceType::RenderTexture2DArrayResource) {
		return std::static_pointer_cast<RenderTexture2DArrayResource>(color_attachments[index].resource);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DCubemapResource> RenderFrameBufferDescriptor::GetColorAttachmentAsTextureCubemap(int index) const
{
	if (color_attachments[index].resource->GetResourceType() == RenderResourceType::RenderTexture2DCubemapResource) {
		return std::static_pointer_cast<RenderTexture2DCubemapResource>(color_attachments[index].resource);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DResource> RenderFrameBufferDescriptor::GetDepthAttachmentAsTexture() const
{
	if (depth_stencil_attachment.resource && depth_stencil_attachment.resource->GetResourceType() == RenderResourceType::RenderTexture2DResource) {
		return std::static_pointer_cast<RenderTexture2DResource>(depth_stencil_attachment.resource);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DArrayResource> RenderFrameBufferDescriptor::GetDepthAttachmentAsTextureArray() const
{
	if (depth_stencil_attachment.resource && depth_stencil_attachment.resource->GetResourceType() == RenderResourceType::RenderTexture2DArrayResource) {
		return std::static_pointer_cast<RenderTexture2DArrayResource>(depth_stencil_attachment.resource);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DCubemapResource> RenderFrameBufferDescriptor::GetDepthAttachmentAsTextureCubemap() const 
{
	if (depth_stencil_attachment.resource && depth_stencil_attachment.resource->GetResourceType() == RenderResourceType::RenderTexture2DCubemapResource) {
		return std::static_pointer_cast<RenderTexture2DCubemapResource>(depth_stencil_attachment.resource);
	}
	return nullptr;
}

int RenderFrameBufferDescriptor::GetColorAttachmentMipLevel(int index)
{
	if (color_attachments[index].resource) {
		return depth_stencil_attachment.level;
	}
	return -1;
}

int RenderFrameBufferDescriptor::GetDepthAttachmentMipLevel()
{
	if (depth_stencil_attachment.resource) {
		return depth_stencil_attachment.level;
	}
	return -1;
}
