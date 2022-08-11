#pragma once 
#include <Renderer/RendererDefines.h>
#include <Renderer/ShaderManager.h>
#include <Renderer/RootSignature.h>
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

	PipelineDescriptor(const PipelineDescriptor& desc) : layout(desc.layout), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode), blend_functions(desc.blend_functions), blend_equation(desc.blend_equation), cull_mode(desc.cull_mode)
	{

	}

	const VertexLayout& GetLayout() const {
		return *layout;
	}

	const RootSignature& GetSignature() const {
		return shader->GetRootSignature();
	}

	const RenderViewport& GetViewport() const {
		return viewport;
	}

	const RenderScissorRect& GetScissorRect() const {
		return scissor_rect;
	}

	const std::shared_ptr<Shader> GetShader() const {
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

	const BlendEquation& GetBlendEquation() const {
		return blend_equation;
	}

	const CullMode& GetCullMode() const {
		return cull_mode;
	}


public:

	VertexLayout* layout = nullptr;
	std::shared_ptr<Shader> shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
	PipelineBlendFunctions blend_functions;
	BlendEquation blend_equation = BlendEquation::ADD;
	CullMode cull_mode = CullMode::BACK;
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

	const RootSignature& GetSignature() const {
		return shader->GetRootSignature();
	}

	const RenderViewport& GetViewport() const {
		return viewport;
	}

	const RenderScissorRect& GetScissorRect() const {
		return scissor_rect;
	}

	const std::shared_ptr<Shader> GetShader() const {
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

	const BlendEquation& GetBlendEquation() const {
		return blend_equation;
	}

	const CullMode& GetCullMode() const {
		return cull_mode;
	}


protected:
	Pipeline(const PipelineDescriptor& desc) : layout(desc.layout), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode), blend_functions(desc.blend_functions), blend_equation(desc.blend_equation),cull_mode(desc.cull_mode)
	{

	}
	Pipeline(PipelineDescriptor&& desc) : layout(desc.layout), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode), blend_equation(desc.blend_equation), blend_functions(desc.blend_functions), cull_mode(desc.cull_mode)
	{

	}
	VertexLayout* layout = nullptr;
	std::shared_ptr<Shader> shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
	PipelineBlendFunctions blend_functions;
	BlendEquation blend_equation = BlendEquation::ADD;
	CullMode cull_mode = CullMode::BACK;
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
	

private:


};
