#include "RenderCommandList.h"
#include <Renderer/Renderer.h>
#include <Renderer/Renderer3D/MaterialManager.h>

#ifdef OpenGL_API
#include "platform/OpenGL/OpenGLRenderCommandList.h"
#elif defined Vulkan_API
#include "platform/Vulkan/VulkanRenderCommandList.h"
#endif



RenderCommandList::RenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc)
    :m_Renderer(renderer), m_Alloc(alloc)
{
}

RenderCommandList* RenderCommandList::CreateQueue(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc){
#ifdef OpenGL_API
    return new OpenGLRenderCommandList(renderer, alloc);
#elif defined Vulkan_API
    return new VulkanRenderCommandList(renderer, alloc);
#endif
}


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