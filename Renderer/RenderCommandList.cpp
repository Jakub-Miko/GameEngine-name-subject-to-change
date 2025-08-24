#include "RenderCommandList.h"
#include <Renderer/Renderer.h>
#include <Renderer/Renderer3D/MaterialManager.h>

#ifdef OpenGL_API
#include "platform/OpenGL/OpenGLRenderCommandList.h"
#elif defined Vulkan_API
#include "platform/Vulkan/VulkanRenderCommandList.h"
#endif


std::vector<Material::MaterialParameter>& RenderCommandList::GetMutableMaterialParameters(Material* material)
{
    return material->parameters;
}

Material::Material_status& RenderCommandList::GetMutableMaterialStatus(Material* material)
{
    return material->status;
}

RenderDescriptorAllocationHandle& RenderCommandList::GetMutableMaterialDescriptorTable(Material* material)
{
    return material->descriptor_table;
}

std::shared_ptr<RenderBufferResource> RenderCommandList::GetMaterialConstantBuffer(Material* material)
{
    return material->constant_buffer;
}