#pragma once
#include <memory>

#include "RenderResource.h"

struct RenderResourceStoreDescriptor {
    RootDescriptorType resource_descriptor_type;
    RenderState default_image_resource_state = RenderState::COMMON;
    bool buffer_is_read_only = false;
    uint32_t max_resource_count = 1024;
};

class RenderResourceStore {
public:
    virtual uint32_t AttachResource(std::shared_ptr<RenderResource> resource) = 0;
    virtual bool DeattachResource(std::shared_ptr<RenderResource> resource) = 0;
    virtual bool IsReadOnly() = 0;
    virtual ~RenderResourceStore() = default;

    const RenderResourceStoreDescriptor& GetDescriptor() const { return descriptor; }
protected:
    RenderResourceStore(const RenderResourceStoreDescriptor& desc) : descriptor(desc) {}

    RenderResourceStoreDescriptor descriptor;
};
