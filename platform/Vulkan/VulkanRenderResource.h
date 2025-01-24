#pragma once 
#include <Renderer/RenderResource.h>
#include <vulkan/vulkan.h>
#include "VulkanRenderContext.h"

class VulkanRenderResourceManager;

class VulkanRenderResource : public RenderResourceExtension {
public:
	VulkanRenderResource() = default;
	virtual ~VulkanRenderResource() {};

	uint64_t read_timeline = 0;
	uint64_t write_timeline = 0; 
};

class VulkanRenderBufferResource : public RenderBufferResource, public VulkanRenderResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;
	

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };

private:
	virtual ~VulkanRenderBufferResource();

	VulkanRenderBufferResource(const RenderBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED)
		: RenderBufferResource(desc,initial_state) {
		read_timeline = -1;
		write_timeline = -1;
	}

	VulkanRenderBufferResource()
		: RenderBufferResource(RenderBufferDescriptor(), RenderState::UNINITIALIZED) {
		read_timeline = -1;
		write_timeline = -1;
	}

	VulkanRenderBufferResource(const VulkanRenderBufferResource& ref) = delete;
	VulkanRenderBufferResource& operator=(const VulkanRenderBufferResource& ref) = delete;

	VulkanRenderBufferResource(VulkanRenderBufferResource&& ref);
	VulkanRenderBufferResource& operator=(VulkanRenderBufferResource&& ref);


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

class VulkanRenderTexture2DResource : public RenderTexture2DResource, public VulkanRenderResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;



	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };


private:
	virtual ~VulkanRenderTexture2DResource();
	VulkanRenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED)
		: RenderTexture2DResource(desc, initial_state) {

	}
	VkImage image;
	VmaAllocation alloc;
};

class VulkanRenderTexture2DArrayResource : public RenderTexture2DArrayResource, public VulkanRenderResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };


private:
	VulkanRenderTexture2DArrayResource(const RenderTexture2DArrayDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DArrayResource(desc, initial_state) {

	}

	virtual ~VulkanRenderTexture2DArrayResource();

	VkImage image;
	VmaAllocation alloc;
};

class VulkanRenderTexture2DCubemapResource : public RenderTexture2DCubemapResource, public VulkanRenderResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };


private:
	VulkanRenderTexture2DCubemapResource(const RenderTexture2DCubemapDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DCubemapResource(desc, initial_state) {

	}

	~VulkanRenderTexture2DCubemapResource();

	VkImage image;
	VmaAllocation alloc;
};


class VulkanRenderFrameBufferResource : public RenderFrameBufferResource {
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

};