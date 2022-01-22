#pragma once 
#include <memory>
#include "RenderResource.h"
#include <Renderer/Renderer.h>

class RenderResourceManager {
public:
	
	static void Initialize();
	static RenderResourceManager* Get();
	static void Shutdown();

	virtual std::shared_ptr<RenderBufferResource> CreateBuffer(const RenderBufferDescriptor& buffer_desc) = 0;
	virtual void UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset) = 0;

	virtual std::shared_ptr<RenderTexture2DResource> CreateTexture(const RenderTexture2DDescriptor& buffer_desc) = 0;
	virtual void UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y) = 0;


	virtual ~RenderResourceManager() {};

private:
	static RenderResourceManager* instance;
};