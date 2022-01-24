#include "OpenGLRenderResource.h"
#include <stdexcept>

void* OpenGLRenderBufferResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void OpenGLRenderBufferResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

void OpenGLRenderBufferResource::SetRenderId(unsigned int id)
{
	if (render_state == RenderState::UNINITIALIZED) {
		render_id = id;
	}
	else {
		throw std::runtime_error("Trying to Initialized resource that has already been initialized");
	}
}

void OpenGLRenderBufferResource::SetVAOId(unsigned int id)
{
	if (descriptor.usage == RenderBufferUsage::VERTEX_BUFFER && extra_id == 0) {
		extra_id = id;
	}
	else {
		throw std::runtime_error("VaoId can only be used by vertex buffer with empty extra_id");
	}
}

void* OpenGLRenderTexture2DResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void OpenGLRenderTexture2DResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

void OpenGLRenderTexture2DResource::SetRenderId(unsigned int id)
{
	if (render_state == RenderState::UNINITIALIZED) {
		render_id = id;
	}
	else {
		throw std::runtime_error("Trying to Initialized resource that has already been initialized");
	}
}

OpenGLTextureSampler::OpenGLTextureSampler(const TextureSamplerDescritor& desc) : TextureSampler(desc)
{

}

void* OpenGLRenderFrameBufferResource::Map()
{
	throw std::runtime_error("Not Implemented");
}

void OpenGLRenderFrameBufferResource::UnMap()
{
	throw std::runtime_error("Not Implemented");
}

void OpenGLRenderFrameBufferResource::SetRenderId(unsigned int id)
{
	if (render_state == RenderState::UNINITIALIZED) {
		render_id = id;
	}
	else {
		throw std::runtime_error("Trying to Initialized resource that has already been initialized");
	}
}
