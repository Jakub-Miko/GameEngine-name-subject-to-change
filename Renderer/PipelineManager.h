#pragma once 
#include <Renderer/RendererDefines.h>
#include <Renderer/ShaderManager.h>
#include <string>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>
#include <memory>

struct VertexLayout;
class RootSignature;

struct PipelineBlendFunctions {
	BlendFunction srcRGB = BlendFunction::ONE, dstRGB = BlendFunction::ZERO, srcAlpha = BlendFunction::ONE, dstAlpha = BlendFunction::ZERO;

	bool operator==(const PipelineBlendFunctions& other) const {
		return srcRGB == other.srcRGB && srcAlpha == other.srcAlpha && dstRGB == other.dstRGB && dstAlpha == other.dstAlpha;
	}

	bool operator!=(const PipelineBlendFunctions& other) const {
		return !operator==(other);
	}

};

struct PipelineDescriptor {
	
	friend class Pipeline;

	PipelineDescriptor() = default;

	PipelineDescriptor(const PipelineDescriptor& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode), blend_functions(blend_functions)
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

	const PipelineBlendFunctions& GetPipelineBlendFunctions() const {
		return blend_functions;
	}
	
	const PrimitivePolygonRenderMode& GetPrimitivePolygonRenderMode() const {
		return polygon_render_mode;
	}

public:

	VertexLayout* layout = nullptr;
	RootSignature* signature = nullptr;
	const Shader* shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
	PipelineBlendFunctions blend_functions;
	PrimitivePolygonRenderMode polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
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
	
	const PipelineBlendFunctions& GetPipelineBlendFunctions() const {
		return blend_functions;
	}

	const PipelineFlags& GetPipelineFlags() const {
		return flags;
	}

	const PrimitivePolygonRenderMode& GetPrimitivePolygonRenderMode() const {
		return polygon_render_mode;
	}

protected:
	Pipeline(const PipelineDescriptor& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode), blend_functions(desc.blend_functions)
	{

	}
	Pipeline(PipelineDescriptor&& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode)
	{

	}
	VertexLayout* layout = nullptr;
	RootSignature* signature = nullptr;
	const Shader* shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
	PipelineBlendFunctions blend_functions;
	PrimitivePolygonRenderMode polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
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

	virtual std::shared_ptr<Pipeline> CreatePipeline(const PipelineDescriptor& desc) = 0;
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
