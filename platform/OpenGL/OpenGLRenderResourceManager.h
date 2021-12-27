#pragma once
#include <Renderer/RenderResourceManager.h>
#include "OpenGLRenderResource.h"
#include <Utilities/MemoryManagement/include/MultiPool.h>
#include <mutex>
#include <memory>

class OpenGLRenderResourceManager : public RenderResourceManager {
public:
	friend RenderResourceManager;
	virtual std::shared_ptr<RenderBufferResource> CreateBuffer(const RenderBufferDescriptor& buffer_desc) override;
	virtual void UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset) override;


private:
	OpenGLRenderResourceManager();
	~OpenGLRenderResourceManager();

	void ReturnBufferResource(RenderBufferResource* resource);

private:
	MultiPool<std::allocator<void>, true, false> ResourcePool;
	std::mutex ResourceMutex;
};