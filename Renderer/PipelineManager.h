#pragma once 
#include <Renderer/RendererDefines.h>
#include <Renderer/ShaderManager.h>
#include <string>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

struct VertexLayout;
class RootSignature;


struct PipelineDescriptor {
	
	friend class Pipeline;

	PipelineDescriptor() = default;

	PipelineDescriptor(const PipelineDescriptor& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags)
	{

	}

	const VertexLayout& GetLayout() const {
		return *layout;
	}

	const RootSignature* GetSignature() const {
		return signature;
	}

	const RenderViewport& GetViewport() const {
		return viewport;
	}

	const RenderScissorRect& GetScissorRect() const {
		return scissor_rect;
	}

	const Shader* GetShader() const {
		return shader;
	}

	const PipelineFlags& GetPipelineFlags() const {
		return flags;
	}
	

public:

	VertexLayout* layout = nullptr;
	RootSignature* signature = nullptr;
	const Shader* shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
};

using PipelineState = PipelineDescriptor;

class Pipeline {
public:
	virtual ~Pipeline() { }
	virtual RootBinding GetBindingId(const std::string& name) = 0;

	const VertexLayout& GetLayout() const {
		return *layout;
	}

	const RootSignature* GetSignature() const {
		return signature;
	}

	const RenderViewport& GetViewport() const {
		return viewport;
	}

	const RenderScissorRect& GetScissorRect() const {
		return scissor_rect;
	}

	const Shader* GetShader() const {
		return shader;
	}
	
	const PipelineFlags& GetPipelineFlags() const {
		return flags;
	}

protected:
	Pipeline(const PipelineDescriptor& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags)
	{

	}
	Pipeline(PipelineDescriptor&& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags)
	{

	}
	VertexLayout* layout = nullptr;
	RootSignature* signature = nullptr;
	const Shader* shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
};


class PipelineManager {
public:
	
	template<typename U>
	friend struct VertexLayoutFactory;

	template<typename U>
	friend struct RootSignatureFactory;

	static void Initialize();
	static PipelineManager* Get();
	static void Shutdown();

	virtual Pipeline* CreatePipeline(const PipelineDescriptor& desc) = 0;
	virtual ~PipelineManager();

	PipelineManager();

private:
	static PipelineManager* instance;
	
	void AddLayout(VertexLayout* layout);
	void AddSignature(RootSignature* signature);

private:

	std::mutex m_Signatures_mutex;
	std::mutex m_Layouts_mutex;
	std::vector<RootSignature*> m_Signatures;
	std::vector<VertexLayout*> m_Layouts;

};
