#include "RenderDescriptorHeapBlock.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderDescriptorHeapBlock.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderDescriptorHeapBlock.h>
#endif

RenderDescriptorHeapBlock* RenderDescriptorHeapBlock::CreateHeapBlock(size_t size)
{
#ifdef OpenGL_API
    return new OpenGLRenderDescriptorHeapBlock(size);
#elif defined Vulkan_API
    return new VulkanRenderDescriptorHeapBlock(size);
#endif
}
