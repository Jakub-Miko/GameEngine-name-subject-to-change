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