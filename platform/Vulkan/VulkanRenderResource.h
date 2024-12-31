#pragma once 
#include <Renderer/RenderResource.h>
#include <vulkan/vulkan.h>
#include "VulkanRenderContext.h"

class VulkanRenderResourceManager;

class VulkanRendeResourceStateExtension {
public:
	uint64_t read_timeline = 0;
	uint64_t write_timeline = 0;
};

class VulkanRenderBufferResource : public RenderBufferResource, VulkanRendeResourceStateExtension {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;
	


	VulkanRenderBufferResource(const RenderBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED)
		: RenderBufferResource(desc,initial_state) {

	}

	~VulkanRenderBufferResource() {}

private:

	VkBuffer buffer;
	VmaAllocation alloc;
};


class VulkanTextureSampler : public TextureSampler {
public:
	friend TextureSampler;

	virtual ~VulkanTextureSampler();

private:
	VulkanTextureSampler(const TextureSamplerDescritor& desc);

	VkSampler sampler;
};

class VulkanRenderTexture2DResource : public RenderTexture2DResource, VulkanRendeResourceStateExtension {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;


	VulkanRenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED)
		: RenderTexture2DResource(desc, initial_state) {

	}

	virtual ~VulkanRenderTexture2DResource() {}

private:
	VkImage texture;
	VmaAllocation alloc;
};

class VulkanRenderTexture2DArrayResource : public RenderTexture2DArrayResource, VulkanRendeResourceStateExtension {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;



	VulkanRenderTexture2DArrayResource(const RenderTexture2DArrayDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DArrayResource(desc, initial_state) {

	}

	virtual ~VulkanRenderTexture2DArrayResource() {}

private:

	VkImage image;

};

class VulkanRenderTexture2DCubemapResource : public RenderTexture2DCubemapResource, VulkanRendeResourceStateExtension {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;


	VulkanRenderTexture2DCubemapResource(const RenderTexture2DCubemapDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DCubemapResource(desc, initial_state) {

	}

	~VulkanRenderTexture2DCubemapResource() {}

private:

	VkImage image;

};


class VulkanRenderFrameBufferResource : public RenderFrameBufferResource, VulkanRendeResourceStateExtension {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;


	VulkanRenderFrameBufferResource(const VulkanRenderFrameBufferResource& other) = delete;

	VulkanRenderFrameBufferResource(const RenderFrameBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderFrameBufferResource(desc, initial_state) {

	}

	virtual ~VulkanRenderFrameBufferResource() {}

private:

	
	VkFramebuffer framebuffer;

};