#pragma once 
#include <Renderer/RenderResource.h>
#include <vulkan/vulkan.h>
#include "VulkanRenderContext.h"

class VulkanRenderResourceManager;

class VulkanRenderResource : public RenderResourceExtension {
public:
	VulkanRenderResource() = default;
	virtual ~VulkanRenderResource() {};
	virtual RenderState GetDefaultState() { return RenderState::COMMON;  };
	virtual bool IsTexture() override { return false;  };

	uint64_t read_timeline = 0;
	uint64_t write_timeline = 0; 
private:
	/**
	 * Override this instead of using the destructor, these resources are just cpu handles and should not directly destroy the underlying resource when destroyed.
	 * 
	 * When the underlying resource is destroyed is determined by the RenderResourceManager. At that time the RenderResourceManager calls this method(it doesnt have to if the resource is not managed by it)
	 */
	friend VulkanRenderResourceManager;
	virtual void DestroyResource() {};
};

class VulkanRenderTextureResource : public VulkanRenderResource {
public:
	RenderState default_state = RenderState::UNINITIALIZED;
	virtual RenderState GetDefaultState() override { return default_state; };
	virtual bool IsTexture() override { return true; };
	virtual VkImageView GetImageView() = 0;
	virtual VkImage GetImage() = 0;
	virtual TextureFormat GetFormat() = 0;
	virtual glm::uvec2 GetResolution() = 0;
	virtual TextureUsage GetUsage() = 0;
	virtual uint32_t GetArrayLayerCount() = 0;
};

class VulkanRenderBufferResource : public RenderBufferResource, public VulkanRenderResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;
	

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };

private:
	virtual ~VulkanRenderBufferResource();
	virtual void DestroyResource() override;

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

	VulkanRenderBufferResource(VulkanRenderBufferResource&& ref) = delete;
	VulkanRenderBufferResource& operator=(VulkanRenderBufferResource&& ref) = delete;


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

class VulkanRenderTexture2DResource : public RenderTexture2DResource, public VulkanRenderTextureResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	virtual VkImage GetImage() override { return image;  };

	virtual TextureFormat GetFormat() override { return descriptor.format;  };

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };

	virtual uint32_t GetArrayLayerCount() override { return 1; };

	virtual TextureUsage GetUsage() override { return descriptor.usage; };

	virtual VkImageView GetImageView() override { return view;  }

	virtual glm::uvec2 GetResolution() override { return { descriptor.width, descriptor.height }; };

private:
	virtual ~VulkanRenderTexture2DResource();
	virtual void DestroyResource() override;
	VulkanRenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED)
		: RenderTexture2DResource(desc, initial_state) {

	}
	VkImage image;
	VkImageView view;
	VmaAllocation alloc;
};

class VulkanRenderTexture2DArrayResource : public RenderTexture2DArrayResource, public VulkanRenderTextureResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };

	virtual VkImage GetImage() override { return image; };

	virtual TextureFormat GetFormat() override { return descriptor.format; };

	virtual uint32_t GetArrayLayerCount() override { return descriptor.num_of_textures; };

	virtual TextureUsage GetUsage() override { return descriptor.usage; };

	virtual VkImageView GetImageView() override { return view; }

	virtual glm::uvec2 GetResolution() override { return { descriptor.width, descriptor.height }; };

private:
	VulkanRenderTexture2DArrayResource(const RenderTexture2DArrayDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DArrayResource(desc, initial_state) {

	}
	virtual void DestroyResource() override;
	virtual ~VulkanRenderTexture2DArrayResource();

	VkImage image;
	VkImageView view;
	VmaAllocation alloc;
};

class VulkanRenderTexture2DCubemapResource : public RenderTexture2DCubemapResource, public VulkanRenderTextureResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	virtual RenderResourceExtension* GetExtensionData() override { return static_cast<RenderResourceExtension*>(this); };
	
	virtual VkImage GetImage() override { return image; };

	virtual TextureFormat GetFormat() override { return descriptor.format; };

	virtual uint32_t GetArrayLayerCount() override { return 6; };

	virtual TextureUsage GetUsage() override { return descriptor.usage; };

	virtual VkImageView GetImageView() override { return view; }

	virtual glm::uvec2 GetResolution() override { return { descriptor.res, descriptor.res }; };

private:
	VulkanRenderTexture2DCubemapResource(const RenderTexture2DCubemapDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DCubemapResource(desc, initial_state) {

	}
	virtual void DestroyResource() override;
	~VulkanRenderTexture2DCubemapResource();

	VkImage image;
	VkImageView view;
	VmaAllocation alloc;
};


class VulkanRenderFrameBufferResource : public RenderFrameBufferResource {
public:
	friend VulkanRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	VulkanRenderFrameBufferResource(const VulkanRenderFrameBufferResource& other) = delete;

	VulkanRenderFrameBufferResource(const RenderFrameBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0);

	VkRenderingInfo& GetRenderingInfo() { return rendering_info;  }

	virtual ~VulkanRenderFrameBufferResource() {}

private:
	std::vector<VkRenderingAttachmentInfo> attachment_info_store; //index 0 is depth, other are color
	VkRenderingInfo rendering_info;
};