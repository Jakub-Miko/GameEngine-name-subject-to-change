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

	virtual ~RenderResourceManager() {};

private:
	static RenderResourceManager* instance;
};