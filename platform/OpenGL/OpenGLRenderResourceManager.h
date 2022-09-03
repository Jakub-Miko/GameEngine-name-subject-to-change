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
	virtual void ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size) override;

	virtual std::shared_ptr<RenderTexture2DResource> CreateTexture(const RenderTexture2DDescriptor& buffer_desc) override;
	virtual void UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;
	virtual void GenerateMIPs(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource) override;
	virtual void UploadToTexture2DFromFile(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level = 0) override;
	virtual std::shared_ptr<RenderTexture2DResource> CreateTextureFromFile(RenderCommandList* list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler) override;

	virtual std::shared_ptr<RenderTexture2DArrayResource> CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc) override;
	virtual void UploadDataToTexture2DArray(RenderCommandList* list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;

	virtual std::shared_ptr<RenderTexture2DCubemapResource> CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc) override;
	virtual void UploadDataToTexture2DCubemap(RenderCommandList* list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;

	virtual std::shared_ptr<RenderFrameBufferResource> CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc) override;

	virtual void CreateConstantBufferDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource) override;
	virtual void CreateTexture2DDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource) override;
	virtual void CreateTexture2DArrayDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource) override;
	virtual void CreateTexture2DCubemapDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource) override;

	virtual void CopyFrameBufferDepthAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer) override;

private:
	OpenGLRenderResourceManager();
	~OpenGLRenderResourceManager();

	void ReturnBufferResource(RenderBufferResource* resource);
	void ReturnTexture2DResource(RenderTexture2DResource* resource);
	void ReturnTexture2DArrayResource(RenderTexture2DArrayResource* resource);
	void ReturnFrameBufferResource(RenderFrameBufferResource* resource);
	void ReturnTexture2DCubemapResource(RenderTexture2DCubemapResource* resource);

private:
	MultiPool<std::allocator<void>, true, false> ResourcePool;
	std::mutex ResourceMutex;
};