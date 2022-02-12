#include "RenderDescriptorHeapBlock.h"
#include <platform/OpenGL/OpenGLRenderDescriptorHeapBlock.h>

RenderDescriptorHeapBlock* RenderDescriptorHeapBlock::CreateHeapBlock(size_t size)
{
    return new OpenGLRenderDescriptorHeapBlock(size);
}
