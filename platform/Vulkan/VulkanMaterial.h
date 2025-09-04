#pragma once
#include <Renderer/MaterialManager.h>
#include "VulkanRenderDescriptorHeap.h"

class VulkanMaterialTemplate : public MaterialTemplate {
public:
    VulkanMaterialTemplate(const MaterialLayout& layout, const std::string& name, Private dummy);

    virtual std::shared_ptr<Material> CreateMaterial() override;

    virtual ~VulkanMaterialTemplate() {};

    VulkanRenderDescriptorHeap& GetAllocator()  {
        return *material_allocator;
    }
   

private:
    std::unique_ptr<VulkanRenderDescriptorHeap> material_allocator;
};

class VulkanMaterial : public Material {
public:
    VulkanMaterial() = default;
    VulkanMaterial(std::shared_ptr<MaterialTemplate> material_template);
    
    virtual ~VulkanMaterial() {}

    virtual void UpdateValues(std::shared_ptr<RenderCommandList>  command_list) override;

    VulkanRenderDescriptorTable GetDescriptorTable() const {
        return descriptor_table;
    }

    std::shared_ptr<RenderBufferResource> GetConstantBuffer() const {
        return constant_buffer;
    }

    // used to replace the descriptor table with a new one which can be modified without influencing previously submitted work.
    void ResetDescriptorTable();

private:
    VulkanRenderDescriptorTable descriptor_table;
    std::shared_ptr<RenderBufferResource> constant_buffer = nullptr;
};