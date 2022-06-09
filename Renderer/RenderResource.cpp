#include "RenderResource.h"
#include <platform/OpenGL/OpenGLRenderResource.h>

std::shared_ptr<TextureSampler> TextureSampler::CreateSampler(const TextureSamplerDescritor& desc)
{
	return std::shared_ptr<OpenGLTextureSampler>(new OpenGLTextureSampler(desc));
}
