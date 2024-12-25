#include "RenderCommandQueue.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRenderCommandQueue.h>
#endif


