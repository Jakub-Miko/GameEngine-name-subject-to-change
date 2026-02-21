#pragma once
#include <memory>

#include "RenderResource.h"

struct RenderResourceStoreDescriptor {
    RootDescriptorType resource_descriptor_type;
    RenderState default_image_resource_state = RenderState::COMMON;
    bool buffer_is_read_only = false;
    uint32_t max_resource_count = 1024;
};

class RenderResourceStoreLayout {
public:
    virtual ~RenderResourceStoreLayout() = default;
protected:
    RenderResourceStoreLayout(RootDescriptorType resource_descriptor_type) : resource_descriptor_type(resource_descriptor_type) {}
    RootDescriptorType resource_descriptor_type;
};

class RenderResourceStore {
public:
    virtual uint32_t AttachResource(std::shared_ptr<RenderResource> resource) = 0;
    virtual bool DeattachResource(std::shared_ptr<RenderResource> resource) = 0;
    virtual bool IsReadOnly() = 0;
    virtual ~RenderResourceStore() = default;

    const RenderResourceStoreDescriptor& GetDescriptor() const { return descriptor; }
    const std::shared_ptr<RenderResourceStoreLayout> GetStoreLayout() const { return store_layout; }
protected:
    RenderResourceStore(std::shared_ptr<RenderResourceStoreLayout> store_layout, const RenderResourceStoreDescriptor& desc) : store_layout(store_layout), descriptor(desc) {}

    RenderResourceStoreDescriptor descriptor;
    std::shared_ptr<RenderResourceStoreLayout> store_layout;
};
