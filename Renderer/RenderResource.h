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


struct RenderTextureDescriptor {

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

protected:
	std::atomic<RenderState> render_state = RenderState::UNINITIALIZED;

};

class RenderBufferResource : public RenderResource {
public:

	RenderBufferResource(const RenderBufferDescriptor& desc, RenderState state) : descriptor(desc), RenderResource(state) {}

	RenderBufferDescriptor GetBufferDescriptor() const {
		return descriptor;
	}

protected:
	RenderBufferDescriptor descriptor;
};

class RenderTextureResource : public RenderResource {
public:

	RenderTextureDescriptor GetBufferDescriptor() const {
		return descriptor;
	}

protected:
	RenderTextureDescriptor descriptor;
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