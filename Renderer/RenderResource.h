#pragma once
#include <atomic>
#include <Core/Hash.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <Renderer/RendererDefines.h>

enum class RenderResourceType : char  {
	INVALID = 0, RenderBufferResource = 1, RenderTexture2DResource = 2, RenderFrameBufferResource = 3, RenderTexture2DArrayResource = 4, RenderTexture2DCubemapResource = 5
};

class RenderResourceDescriptor {
public:
	~RenderResourceDescriptor() {}
};

struct RenderBufferDescriptor {
	RenderBufferDescriptor(size_t size, RenderBufferType type, RenderBufferUsage usage) : buffer_size(size), type(type), usage(usage) {}
	const size_t buffer_size;
	const RenderBufferType type = RenderBufferType::DEFAULT;
	const RenderBufferUsage usage;
};

class RenderResource {
public:
	RenderResource(RenderState state) : render_state(state) {}

	RenderState GetRenderState() const {
		return render_state.load();
	}

	std::atomic<RenderState>& GetRenderStateAtomic() {
		return render_state;
	}

	void SetRenderState(RenderState state) {
		render_state.store(state);
	}

	virtual RenderResourceType GetResourceType() = 0;
	virtual void* Map() = 0;
	virtual void UnMap() = 0;

	virtual ~RenderResource() {};

protected:
	std::atomic<RenderState> render_state = RenderState::UNINITIALIZED;

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
	int width, height;
	TextureFormat format;
	std::shared_ptr<TextureSampler> sampler = nullptr;
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
	TextureFormat format;
	std::shared_ptr<TextureSampler> sampler = nullptr;
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
	TextureFormat format;
	std::shared_ptr<TextureSampler> sampler = nullptr;
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
	std::shared_ptr<RenderTexture2DResource> GetColorAttachmentAsTexture(int index);
	std::shared_ptr<RenderTexture2DArrayResource> GetColorAttachmentAsTextureArray(int index);
	std::shared_ptr<RenderTexture2DCubemapResource> GetColorAttachmentAsTextureCubemap(int index);
	std::shared_ptr<RenderTexture2DResource> GetDepthAttachmentAsTexture();
	std::shared_ptr<RenderTexture2DArrayResource> GetDepthAttachmentAsTextureArray();
	std::shared_ptr<RenderTexture2DCubemapResource> GetDepthAttachmentAsTextureCubemap();

	int GetColorAttachmentMipLevel(int index);
	int GetDepthAttachmentMipLevel();

	struct RenderFrameBufferAttachment {
		int level = 0;
		std::shared_ptr<RenderResource> resource;
	};

	std::vector<RenderFrameBufferAttachment> color_attachments;
	RenderFrameBufferAttachment depth_stencil_attachment;
};

class RenderFrameBufferResource : public RenderResource {
public:

	RenderFrameBufferResource(const RenderFrameBufferDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderFrameBufferDescriptor GetBufferDescriptor() const {
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




