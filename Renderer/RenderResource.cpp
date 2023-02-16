#include "RenderResource.h"
#include <platform/OpenGL/OpenGLRenderResource.h>

std::shared_ptr<TextureSampler> TextureSampler::CreateSampler(const TextureSamplerDescritor& desc)
{
	return std::shared_ptr<OpenGLTextureSampler>(new OpenGLTextureSampler(desc));
}

std::shared_ptr<RenderTexture2DResource> RenderFrameBufferDescriptor::GetColorAttachmentAsTexture(int index)
{
	if (color_attachments[index]->GetResourceType() == RenderResourceType::RenderTexture2DResource) {
		return std::static_pointer_cast<RenderTexture2DResource>(color_attachments[index]);
	}
}

std::shared_ptr<RenderTexture2DArrayResource> RenderFrameBufferDescriptor::GetColorAttachmentAsTextureArray(int index)
{
	if (color_attachments[index]->GetResourceType() == RenderResourceType::RenderTexture2DArrayResource) {
		return std::static_pointer_cast<RenderTexture2DArrayResource>(color_attachments[index]);
	}
}

std::shared_ptr<RenderTexture2DCubemapResource> RenderFrameBufferDescriptor::GetColorAttachmentAsTextureCubemap(int index)
{
	if (color_attachments[index]->GetResourceType() == RenderResourceType::RenderTexture2DCubemapResource) {
		return std::static_pointer_cast<RenderTexture2DCubemapResource>(color_attachments[index]);
	}
}

std::shared_ptr<RenderTexture2DResource> RenderFrameBufferDescriptor::GetDepthAttachmentAsTexture()
{
	if (depth_stencil_attachment && depth_stencil_attachment->GetResourceType() == RenderResourceType::RenderTexture2DResource) {
		return std::static_pointer_cast<RenderTexture2DResource>(depth_stencil_attachment);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DArrayResource> RenderFrameBufferDescriptor::GetDepthAttachmentAsTextureArray()
{
	if (depth_stencil_attachment && depth_stencil_attachment->GetResourceType() == RenderResourceType::RenderTexture2DArrayResource) {
		return std::static_pointer_cast<RenderTexture2DArrayResource>(depth_stencil_attachment);
	}
	return nullptr;
}

std::shared_ptr<RenderTexture2DCubemapResource> RenderFrameBufferDescriptor::GetDepthAttachmentAsTextureCubemap() 
{
	if (depth_stencil_attachment && depth_stencil_attachment->GetResourceType() == RenderResourceType::RenderTexture2DCubemapResource) {
		return std::static_pointer_cast<RenderTexture2DCubemapResource>(depth_stencil_attachment);
	}
	return nullptr;
}
