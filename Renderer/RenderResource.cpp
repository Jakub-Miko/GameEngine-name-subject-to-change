#include "RenderResource.h"
#include <platform/OpenGL/OpenGLRenderResource.h>

TextureSampler* TextureSampler::CreateSampler(const TextureSamplerDescritor& desc)
{
	return new OpenGLTextureSampler(desc);
}
