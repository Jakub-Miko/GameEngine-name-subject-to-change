#pragma once
#include "Renderer/RenderResourceStore.h"
#include <vulkan/vulkan.h>

#include "VulkanDeferredDestruction.h"
#include "VulkanRenderResource.h"

class VulkanRenderResourceStore : public RenderResourceStore, public VulkanDeferredDestruction, public std::enable_shared_from_this<VulkanRenderResourceStore> {
public:
    bool Destroy() override;
    uint32_t AttachResource(std::shared_ptr<RenderResource> resource) override;
    bool DeattachResource(std::shared_ptr<RenderResource> resource) override;
    bool IsReadOnly() override;
    uint64_t GetTimelineValue() const {
        return timeline;
    }

    void SetTimelineValue(uint64_t value) {
        timeline = value;
    }

    ~VulkanRenderResourceStore() override = default;

private:
    friend class VulkanRenderResourceManager;
    explicit VulkanRenderResourceStore(const RenderResourceStoreDescriptor& desc) : RenderResourceStore(desc) {}

    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
    uint32_t last_allocated_binding = 0;
    std::unordered_map<uint32_t,std::shared_ptr<RenderResource>> resource_bindings;
    std::vector<uint32_t> free_indices;
    uint64_t timeline = 0;
};