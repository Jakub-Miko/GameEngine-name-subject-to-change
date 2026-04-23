#pragma once
#include <atomic>
#include <Core/Hash.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <Renderer/RendererDefines.h>

class RenderResourceStore;

enum class RenderResourceType : char  {
	INVALID = 0, RenderBufferResource = 1, RenderTexture2DResource = 2, RenderFrameBufferResource = 3, RenderTexture2DArrayResource = 4, RenderTexture2DCubemapResource = 5
};

enum class RenderCubemapFace : char {
	CUBEMAP_RIGHT = 0, CUBEMAP_LEFT = 1, CUBEMAP_TOP = 2, CUBEMAP_BOTTOM = 3, CUBEMAP_FRONT = 4, CUBEMAP_BACK = 5,
};

class RenderResourceDescriptor {
public:
	~RenderResourceDescriptor() {}
};

enum class RenderBufferCreationFlags : char {
	NONE = 0,
	CREATE_INITIALIZED = (1 << 0)
};

inline RenderBufferCreationFlags operator&(const RenderBufferCreationFlags& first, const RenderBufferCreationFlags& second) {
	return (RenderBufferCreationFlags)((char)first & (char)second);
}

struct RenderBufferDescriptor {
	RenderBufferDescriptor() = default;
	RenderBufferDescriptor(size_t size, RenderBufferType type, RenderBufferUsage usage) : buffer_size(size), type(type), usage(usage) {}
	size_t buffer_size = 0;
	RenderBufferType type = RenderBufferType::DEFAULT;
	RenderBufferUsage usage = RenderBufferUsage::CONSTANT_BUFFER;
};

class RenderResourceExtension {
public:
	virtual bool IsTexture() { return false; };
};

class RenderResource {
public:
	RenderResource(RenderState state) : render_state(state) {}

	RenderState GetRenderState() const {
		return render_state;
	}

	void SetRenderState(RenderState state) {
		render_state = state;
	}

	virtual RenderResourceExtension* GetExtensionData() { return nullptr; };
	virtual RenderResourceType GetResourceType() = 0;
	virtual void* Map() = 0;
	virtual void UnMap() = 0;
	virtual std::shared_ptr<RenderResourceStore> GetResourceStore() { return nullptr; }
	virtual uint32_t GetResourceStoreIndex() { return -1; }

	virtual ~RenderResource() {};

protected:
	RenderState render_state = RenderState::UNINITIALIZED;
};

class RenderBufferResource : public RenderResource {
public:

	RenderBufferResource(const RenderBufferDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderBufferDescriptor GetBufferDescriptor() const {
		return descriptor;
	}

	virtual RenderResourceType GetResourceType() override {
		return RenderResourceType::RenderBufferResource;
	}

	virtual ~RenderBufferResource() {};

protected:
	RenderBufferDescriptor descriptor;
};

struct TextureSamplerDescritor {
	TextureAddressMode AddressMode_U = TextureAddressMode::MIRROR, AddressMode_V = TextureAddressMode::MIRROR, AddressMode_W = TextureAddressMode::MIRROR;
	TextureFilter filter = TextureFilter::LINEAR_MIN_MAG;
	DepthComparisonMode comparison_mode = DepthComparisonMode::DISABLED;
	glm::vec4 border_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float LOD_bias = 0;
	float min_LOD = 0;
	float max_LOD = 15;
	bool enable_anisotropy = false;

	bool operator==(const TextureSamplerDescritor& other)const
	{
		return ((char)AddressMode_U == (char)other.AddressMode_U) &&
			((char)AddressMode_V == (char)other.AddressMode_V) &&
			((char)AddressMode_W == (char)other.AddressMode_W) &&
			((char)filter == (char)other.AddressMode_U) &&
			(border_color == other.border_color) &&
			(min_LOD == other.min_LOD) &&
			(max_LOD == other.max_LOD) &&
			(LOD_bias == other.LOD_bias);
	}

	size_t hash() {
		size_t value = 0;
		hash_combine(value, (int)AddressMode_U);
		hash_combine(value, (int)AddressMode_V);
		hash_combine(value, (int)AddressMode_W);
		hash_combine(value, (int)filter);
		hash_combine(value, border_color.x);
		hash_combine(value, border_color.y);
		hash_combine(value, border_color.z);
		hash_combine(value, border_color.w);
		hash_combine(value, LOD_bias);
		hash_combine(value, max_LOD);
		hash_combine(value, min_LOD);
		return value;
	}

};

template <>
struct std::hash<TextureSamplerDescritor>
{
	std::size_t operator()(const TextureSamplerDescritor& k) const
	{
		size_t value = 0;
		hash_combine(value, (int)k.AddressMode_U);
		hash_combine(value, (int)k.AddressMode_V);
		hash_combine(value, (int)k.AddressMode_W);
		hash_combine(value, (int)k.filter);
		hash_combine(value, k.border_color.x);
		hash_combine(value, k.border_color.y);
		hash_combine(value, k.border_color.z);
		hash_combine(value, k.border_color.w);
		hash_combine(value, k.LOD_bias);
		hash_combine(value, k.max_LOD);
		hash_combine(value, k.min_LOD);
		return value;
	}
};




class TextureSampler {
public:

	TextureSampler(const TextureSamplerDescritor& desc) : descriptor(desc) {}

	virtual ~TextureSampler() {}

	const TextureSamplerDescritor& GetDescriptor() const { return descriptor; }

	static std::shared_ptr<TextureSampler> CreateSampler(const TextureSamplerDescritor& desc);

private:

	TextureSamplerDescritor descriptor;

};


struct RenderTexture2DDescriptor {
	int width = 0, height = 0;
	int mipmap_levels = 1;
	TextureFormat format = TextureFormat::RGBA_32FLOAT;
	std::shared_ptr<TextureSampler> sampler = nullptr;
	TextureUsage usage = TextureUsage::DEFAULT;
};

class RenderTexture2DResource : public RenderResource {
public:

	RenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderTexture2DDescriptor GetBufferDescriptor() const {
		return descriptor;
	}

	virtual RenderResourceType GetResourceType() override {
		return RenderResourceType::RenderTexture2DResource;
	}

	virtual ~RenderTexture2DResource() {};

protected:
	RenderTexture2DDescriptor descriptor;
};

struct RenderTexture2DArrayDescriptor {
	int width, height, num_of_textures;
	int mipmap_levels = 1;
	TextureFormat format;
	std::shared_ptr<TextureSampler> sampler = nullptr;
	TextureUsage usage = TextureUsage::DEFAULT;
};

class RenderTexture2DArrayResource : public RenderResource {
public:

	RenderTexture2DArrayResource(const RenderTexture2DArrayDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderTexture2DArrayDescriptor  GetBufferDescriptor() const {
		return descriptor;
	}

	virtual RenderResourceType GetResourceType() override {
		return RenderResourceType::RenderTexture2DArrayResource;
	}

	virtual ~RenderTexture2DArrayResource() {};

protected:
	RenderTexture2DArrayDescriptor  descriptor;
};


struct RenderTexture2DCubemapDescriptor {
	bool generate_mips = false;
	int res;
	int mipmap_levels = 1;
	TextureFormat format;
	std::shared_ptr<TextureSampler> sampler = nullptr;
	TextureUsage usage = TextureUsage::DEFAULT;
};

class RenderTexture2DCubemapResource : public RenderResource {
public:

	RenderTexture2DCubemapResource(const RenderTexture2DCubemapDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderTexture2DCubemapDescriptor  GetBufferDescriptor() const {
		return descriptor;
	}

	virtual RenderResourceType GetResourceType() override {
		return RenderResourceType::RenderTexture2DCubemapResource;
	}

	virtual ~RenderTexture2DCubemapResource() {};

protected:
	RenderTexture2DCubemapDescriptor  descriptor;
};

struct RenderFrameBufferDescriptor {
	std::shared_ptr<RenderTexture2DResource> GetColorAttachmentAsTexture(int index) const;
	std::shared_ptr<RenderTexture2DArrayResource> GetColorAttachmentAsTextureArray(int index) const;
	std::shared_ptr<RenderTexture2DCubemapResource> GetColorAttachmentAsTextureCubemap(int index) const;
	std::shared_ptr<RenderTexture2DResource> GetDepthAttachmentAsTexture() const;
	std::shared_ptr<RenderTexture2DArrayResource> GetDepthAttachmentAsTextureArray() const;
	std::shared_ptr<RenderTexture2DCubemapResource> GetDepthAttachmentAsTextureCubemap() const;

	int GetColorAttachmentMipLevel(int index);
	int GetDepthAttachmentMipLevel();

	struct RenderFrameBufferAttachment {
		int level = 0;
		std::shared_ptr<RenderResource> resource = nullptr;
	};

	std::vector<RenderFrameBufferAttachment> color_attachments;
	RenderFrameBufferAttachment depth_stencil_attachment;
};

class RenderFrameBufferResource : public RenderResource {
public:

	RenderFrameBufferResource(const RenderFrameBufferDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	const RenderFrameBufferDescriptor& GetBufferDescriptor() const {
		return descriptor;
	}

	virtual RenderResourceType GetResourceType() override {
		return RenderResourceType::RenderFrameBufferResource;
	}

	virtual ~RenderFrameBufferResource() {};

protected:
	friend class RenderResourceManager;
	RenderFrameBufferDescriptor descriptor;
};




