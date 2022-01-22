#pragma once
#include <atomic>
#include <vector>
#include <memory>
#include <Renderer/RendererDefines.h>

enum class RenderState : unsigned char
{
	UNINITIALIZED = 0, READ = 1, WRITE = 2, COMMON = 3, EMPTY = 4, IN_USE_VERTEX_BUFFER = 5, IN_USE_INDEX_BUFFER = 6
};

enum class RenderBufferType : unsigned char
{
	DEFAULT = 0, UPLOAD = 1
};

enum class RenderBufferUsage : unsigned char
{
	VERTEX_BUFFER = 0, INDEX_BUFFER = 1, CONSTANT_BUFFER = 2
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

	virtual ~RenderBufferResource() {};

protected:
	RenderBufferDescriptor descriptor;
};

struct TextureSamplerDescritor {
	TextureAddressMode AddressMode_U, AddressMode_V, AddressMode_W;
	TextureFilter filter;
	float LOD_bias;
	glm::vec4 border_color;
	float min_LOD;
	float max_LOD;
};



class TextureSampler {
public:

	TextureSampler(const TextureSamplerDescritor& desc) : descriptor(desc) {}

	virtual ~TextureSampler() {}

	const TextureSamplerDescritor& GetDescriptor() const { return descriptor; }

	static TextureSampler* CreateSampler(const TextureSamplerDescritor& desc);

private:

	TextureSamplerDescritor descriptor;

};


struct RenderTexture2DDescriptor {
	int width, height;
	TextureFormat format;
	TextureSampler* sampler = nullptr;
};

class RenderTexture2DResource : public RenderResource {
public:

	RenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderTexture2DDescriptor GetBufferDescriptor() const {
		return descriptor;
	}

	virtual ~RenderTexture2DResource() {};

protected:
	RenderTexture2DDescriptor descriptor;
};



class RenderIndexBufferView {
public:
	RenderIndexBufferView(std::shared_ptr<RenderBufferResource> resource, RenderPrimitiveType type, size_t size) : Resource(resource), type(type), vertex_count(size) {}

	std::shared_ptr<RenderBufferResource> GetResource() const {
		return Resource;
	}

	RenderPrimitiveType GetIndexBufferType() const {
		return type;
	}

	size_t GetVertexCount() const {
		return vertex_count;
	}

private:
	std::shared_ptr<RenderBufferResource> Resource;
	size_t vertex_count;
	RenderPrimitiveType type;
};

