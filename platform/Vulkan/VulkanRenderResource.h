#pragma once 
#include <Renderer/RenderResource.h>
#include <vulkan/vulkan.h>

class VulkanRenderResourceManager;

class VulkanRenderBufferResource : public RenderBufferResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;
	


	VulkanRenderBufferResource(const RenderBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderBufferResource(desc,initial_state) {

	}

	~VulkanRenderBufferResource() {}

private:

	VkBuffer buffer;

};


class VulkanTextureSampler : public TextureSampler {
public:
	friend TextureSampler;

	virtual ~VulkanTextureSampler() {}

private:
	VulkanTextureSampler(const TextureSamplerDescritor& desc);
};

class VulkanRenderTexture2DResource : public RenderTexture2DResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;


	VulkanRenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DResource(desc, initial_state) {

	}

	virtual ~VulkanRenderTexture2DResource() {}

private:

	VkSampler sampler;

};

class VulkanRenderTexture2DArrayResource : public RenderTexture2DArrayResource {
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

class VulkanRenderTexture2DCubemapResource : public RenderTexture2DCubemapResource {
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

	
	VkFramebuffer framebuffer;

};