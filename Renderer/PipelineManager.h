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

struct FramebufferFormat {
	struct ColorAttachmentFormat {
		TextureFormat format = TextureFormat::RGBA_32FLOAT;
	};

	struct DepthAttachmentFormat {
		TextureFormat format = TextureFormat::DEFAULT_DEPTH;
	};

	std::vector<ColorAttachmentFormat> color_attachemt_formats = std::vector<ColorAttachmentFormat>();
	DepthAttachmentFormat depth_attachemt_format = DepthAttachmentFormat();
};

struct GraphicsPipelineDescriptor {
	
	friend class GraphicsPipeline;

	GraphicsPipelineDescriptor() = default;

	GraphicsPipelineDescriptor(const GraphicsPipelineDescriptor& desc) : layout(desc.layout), shader(desc.shader), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		flags(desc.flags), polygon_render_mode(desc.polygon_render_mode), blend_functions(desc.blend_functions), 
		blend_equation(desc.blend_equation), cull_mode(desc.cull_mode), depth_function(desc.depth_function), enable_depth_clip(desc.enable_depth_clip), framebuffer_format(desc.framebuffer_format)
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

	const DepthFunction& GetDepthFunction() const {
		return depth_function;
	}

	bool IsDepthClipEnabled() const {
		return enable_depth_clip;
	}

public:

	VertexLayout* layout = nullptr;
	std::shared_ptr<Shader> shader = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineFlags flags = PipelineFlags::DEFAULT;
	PipelineBlendFunctions blend_functions;
	BlendEquation blend_equation = BlendEquation::ADD;
	DepthFunction depth_function = DepthFunction::LESS;
	CullMode cull_mode = CullMode::NONE;
	FramebufferFormat framebuffer_format = FramebufferFormat();
	PrimitivePolygonRenderMode polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	bool enable_depth_clip = true;
};

using PipelineState = GraphicsPipelineDescriptor;

class Pipeline;

class PipelineNativeExtension {
public:
	virtual ~PipelineNativeExtension() = default;
};

class Pipeline {
public:
	virtual ~Pipeline() { }
	virtual RootBinding GetBindingId(const std::string& name) = 0;

	virtual std::shared_ptr<PipelineNativeExtension> GetPipelineNativeExtension() = 0;

	const RootSignature& GetSignature() const {
		return shader->GetRootSignature();
	}

	const std::shared_ptr<Shader> GetShader() const {
		return shader;
	}

	const PipelineFlags& GetPipelineFlags() const {
		return flags;
	}

protected:
	Pipeline(std::shared_ptr<Shader> shader, PipelineFlags flags) : shader(shader), flags(flags)
	{

	}

	std::shared_ptr<Shader> shader = nullptr;
	PipelineFlags flags = PipelineFlags::DEFAULT;
};

class GraphicsPipeline : public Pipeline {
public:
	virtual ~GraphicsPipeline() { }

	const VertexLayout& GetLayout() const {
		return *layout;
	}

	const RenderViewport& GetViewport() const {
		return viewport;
	}

	const RenderScissorRect& GetScissorRect() const {
		return scissor_rect;
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

	const DepthFunction& GetDepthFunction() const {
		return depth_function;
	}

	bool IsDepthClipEnabled() const {
		return enable_depth_clip;
	}

protected:
	GraphicsPipeline(const GraphicsPipelineDescriptor& desc) : Pipeline(desc.shader, desc.flags), layout(desc.layout), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		polygon_render_mode(desc.polygon_render_mode), blend_functions(desc.blend_functions), blend_equation(desc.blend_equation),cull_mode(desc.cull_mode), depth_function(desc.depth_function), enable_depth_clip(desc.enable_depth_clip)
	{

	}
	GraphicsPipeline(GraphicsPipelineDescriptor&& desc) : Pipeline(desc.shader, desc.flags), layout(desc.layout), viewport(desc.viewport), scissor_rect(desc.scissor_rect),
		polygon_render_mode(desc.polygon_render_mode), blend_equation(desc.blend_equation), blend_functions(desc.blend_functions), cull_mode(desc.cull_mode), depth_function(desc.depth_function), enable_depth_clip(desc.enable_depth_clip)
	{

	}

	VertexLayout* layout = nullptr;
	RenderViewport viewport = RenderViewport();
	RenderScissorRect scissor_rect = RenderScissorRect();
	PipelineBlendFunctions blend_functions;
	BlendEquation blend_equation = BlendEquation::ADD;
	DepthFunction depth_function = DepthFunction::LESS;
	CullMode cull_mode = CullMode::NONE;
	PrimitivePolygonRenderMode polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	bool enable_depth_clip = true;
};

struct ComputePipelineDescriptor {

	friend class Pipeline;

	ComputePipelineDescriptor() = default;

	ComputePipelineDescriptor(const GraphicsPipelineDescriptor& desc) : shader(desc.shader), flags(desc.flags) {

	}

	const RootSignature& GetSignature() const {
		return shader->GetRootSignature();
	}

	const std::shared_ptr<Shader> GetShader() const {
		return shader;
	}

	const PipelineFlags& GetPipelineFlags() const {
		return flags;
	}

public:

	std::shared_ptr<Shader> shader = nullptr;
	PipelineFlags flags = PipelineFlags::DEFAULT;
};

class ComputePipeline : public Pipeline {
public:
	virtual ~ComputePipeline() { }

protected:
	ComputePipeline(const ComputePipelineDescriptor& desc) : Pipeline(desc.shader, desc.flags)
	{

	}

	ComputePipeline(ComputePipelineDescriptor&& desc) : Pipeline(desc.shader, desc.flags)
	{

	}
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

	virtual std::shared_ptr<Pipeline> CreatePipeline(const GraphicsPipelineDescriptor& desc) = 0;
	virtual std::shared_ptr<Pipeline> CreatePipeline(const ComputePipelineDescriptor& desc) = 0;
	virtual ~PipelineManager();

	PipelineManager();

private:
	static PipelineManager* instance;
	

private:


};
