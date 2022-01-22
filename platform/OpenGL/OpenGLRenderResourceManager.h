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

	virtual std::shared_ptr<RenderTexture2DResource> CreateTexture(const RenderTexture2DDescriptor& buffer_desc) override;
	virtual void UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y) override;

private:
	OpenGLRenderResourceManager();
	~OpenGLRenderResourceManager();

	void ReturnBufferResource(RenderBufferResource* resource);
	void ReturnTexture2DResource(RenderTexture2DResource* resource);

private:
	MultiPool<std::allocator<void>, true, false> ResourcePool;
	std::mutex ResourceMutex;
};