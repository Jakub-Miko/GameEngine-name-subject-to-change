#include "PostProcessingPass.h"
#include <Renderer/RootSignature.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/MaterialManager.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderResourceManager.h>

#include "Application.h"
#include "World/Components/CameraComponent.h"

struct PostProcessingPreset;

template<>
struct VertexLayoutFactory<PostProcessingPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv")
				});


			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

struct PostProcessingPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<Pipeline> pipeline_with_overlay;
	std::shared_ptr<RenderBufferResource> vertex_buffer;
	std::shared_ptr<RenderBufferResource> index_buffer;
	bool initialized = false;
};


void PostProcessingPass::InitPostProcessingPassData() {
	GraphicsPipelineDescriptor pipeline_desc = {};
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::DISABLE_DEPTH_WRITE;
	pipeline_desc.depth_function = DepthFunction::ALWAYS;
	pipeline_desc.layout = VertexLayoutFactory<PostProcessingPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/PostProcessingShader.glsl");
	pipeline_desc.framebuffer_format.color_attachemt_formats = {
		{ TextureFormat::BGRA_SRGB }
	};
	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/PostProcessingShaderWithOverlay.glsl");

	data->pipeline_with_overlay = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	struct Vertex {
		glm::vec2 pos;
		glm::vec2 uv;
	};

	Vertex vertecies[4] = {
		Vertex{{-1,-1},{0,0}},
		Vertex{{-1,1},{0,1}},
		Vertex{{1,-1},{1,0}},
		Vertex{{1,1},{1,1}}
	};
	unsigned int indicies[6] = { 0,1,2,1,3,2 };

	RenderBufferDescriptor index_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
	RenderBufferDescriptor vertex_desc(sizeof(vertecies), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
	data->vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_desc);
	data->index_buffer = RenderResourceManager::Get()->CreateBuffer(index_desc);
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();

	RenderResourceManager::Get()->UploadDataToBuffer(list, data->vertex_buffer,(void*)vertecies, sizeof(vertecies),0);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->index_buffer, (void*)indicies, sizeof(indicies),0);
	queue->ExecuteRenderCommandList(list);
	data->initialized = true;
}

PostProcessingPass::PostProcessingPass(const std::string& input_framebuffer, const std::string& input_overlay) : input_framebuffer(input_framebuffer), input_overlay(input_overlay)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void PostProcessingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_framebuffer, RenderPassResourceDescriptor_Access::READ);
	if(!input_overlay.empty()) {
		setup_builder.AddResource<std::shared_ptr<RenderTexture2DResource>>(input_overlay, RenderPassResourceDescriptor_Access::READ);
		enable_overlay_prop = setup_builder.GetProperties()->SetProperty("Enable post process overlay", false).second;
	}
}

void PostProcessingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	PROFILE("PostProcessingPass");
	auto frame_buffer_texure = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_framebuffer);
	auto camera_ent = Application::GetWorld().GetPrimaryEntity();
	auto& camera_component = Application::GetWorld().GetComponent<CameraComponent>(camera_ent);
	float exposure = camera_component.exposure;

	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	bool has_overlay = enable_overlay_prop ? enable_overlay_prop->GetValueTyped() : false;

	list->SetPipeline(has_overlay ? data->pipeline_with_overlay : data->pipeline);
	list->SetDefaultRenderTarget();
	list->SetVertexBuffer(data->vertex_buffer);
	list->SetIndexBuffer(data->index_buffer);
	list->SetTexture2D("Color", frame_buffer_texure.get()->GetBufferDescriptor().GetColorAttachmentAsTexture(0));

	if (has_overlay) {
		auto overlay_buffer = resource_manager.GetResource<std::shared_ptr<RenderTexture2DResource>>(input_overlay);
		list->SetTexture2D("Overlay", overlay_buffer);
	}

	list->SetPushConstantRange(&exposure, sizeof(float));
	list->Draw(6);

	queue->ExecuteRenderCommandList(list);
}

PostProcessingPass::~PostProcessingPass()
{
	delete data;
}
