#pragma once 
#include <Renderer/RenderResource.h>
class OpenGLRenderResourceManager;

class OpenGLRenderBufferResource : public RenderBufferResource {
public:
	friend OpenGLRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;
	
	void SetRenderId(unsigned int id);

	void SetVAOId(unsigned int id);

	unsigned int GetVAOId() const {
		return extra_id;
	}

	unsigned int GetRenderId() const {
		return render_id;
	}

	OpenGLRenderBufferResource(const RenderBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderBufferResource(desc,initial_state), render_id(render_id) {

	}

	~OpenGLRenderBufferResource() {}

private:

	unsigned int render_id = 0;

	//Used Only by Vertex Buffers as VAO id, otherwise 0.
	unsigned int extra_id = 0;

};


class OpenGLTextureSampler : public TextureSampler {
public:
	friend TextureSampler;

	virtual ~OpenGLTextureSampler() {}

private:
	OpenGLTextureSampler(const TextureSamplerDescritor& desc);
};

class OpenGLRenderTexture2DResource : public RenderTexture2DResource {
public:
	friend OpenGLRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	void SetRenderId(unsigned int id);

	unsigned int GetRenderId() const {
		return render_id;
	}

	OpenGLRenderTexture2DResource(const RenderTexture2DDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DResource(desc, initial_state), render_id(render_id) {

	}

	virtual ~OpenGLRenderTexture2DResource() {}

private:

	unsigned int render_id = 0;

};

class OpenGLRenderTexture2DArrayResource : public RenderTexture2DArrayResource {
public:
	friend OpenGLRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	void SetRenderId(unsigned int id);

	unsigned int GetRenderId() const {
		return render_id;
	}

	OpenGLRenderTexture2DArrayResource(const RenderTexture2DArrayDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DArrayResource(desc, initial_state), render_id(render_id) {

	}

	virtual ~OpenGLRenderTexture2DArrayResource() {}

private:

	unsigned int render_id = 0;

};

class OpenGLRenderTexture2DCubemapResource : public RenderTexture2DCubemapResource {
public:
	friend OpenGLRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	void SetRenderId(unsigned int id);

	unsigned int GetRenderId() const {
		return render_id;
	}

	OpenGLRenderTexture2DCubemapResource(const RenderTexture2DCubemapDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderTexture2DCubemapResource(desc, initial_state), render_id(render_id) {

	}

	~OpenGLRenderTexture2DCubemapResource() {}

private:

	unsigned int render_id = 0;

};


class OpenGLRenderFrameBufferResource : public RenderFrameBufferResource {
public:
	friend OpenGLRenderResourceManager;

	virtual void* Map() override;

	virtual void UnMap() override;

	void SetRenderId(unsigned int id);

	unsigned int GetRenderId() const {
		return render_id;
	}

	OpenGLRenderFrameBufferResource(const OpenGLRenderFrameBufferResource& other) = delete;

	OpenGLRenderFrameBufferResource(const RenderFrameBufferDescriptor& desc, RenderState initial_state = RenderState::UNINITIALIZED, unsigned int render_id = 0)
		: RenderFrameBufferResource(desc, initial_state), render_id(render_id) {

	}

	virtual ~OpenGLRenderFrameBufferResource() {}

private:

	unsigned int render_id = 0;

};