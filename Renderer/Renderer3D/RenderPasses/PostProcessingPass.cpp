#include "PostProcessingPass.h"
#include <Renderer/RootSignature.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderResourceManager.h>

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
	std::shared_ptr<RenderBufferResource> vertex_buffer;
	std::shared_ptr<RenderBufferResource> index_buffer;
	bool initialized = false;
};


void PostProcessingPass::InitPostProcessingPassData() {
	PipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.depth_function = DepthFunction::ALWAYS;
	pipeline_desc.layout = VertexLayoutFactory<PostProcessingPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/PostProcessingShader.glsl");

	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

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

	RenderResourceManager::Get()->ReallocateAndUploadBuffer(list, data->vertex_buffer,(void*)vertecies, sizeof(vertecies));
	RenderResourceManager::Get()->ReallocateAndUploadBuffer(list, data->index_buffer, (void*)indicies, sizeof(indicies));
	queue->ExecuteRenderCommandList(list);
	data->initialized = true;
}

PostProcessingPass::PostProcessingPass(const std::string& input_buffer_name) : input_buffer_name(input_buffer_name) 
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void PostProcessingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer_name, RenderPassResourceDescriptor_Access::READ);
}

void PostProcessingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	auto frame_buffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer_name);

	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();

	list->SetPipeline(data->pipeline);
	list->SetDefaultRenderTarget();
	list->SetVertexBuffer(data->vertex_buffer);
	list->SetIndexBuffer(data->index_buffer);
	list->SetTexture2D("Color", frame_buffer->GetBufferDescriptor().color_attachments[0]);
	list->SetTexture2D("Depth", frame_buffer->GetBufferDescriptor().depth_stencil_attachment);
	list->Draw(6);

	queue->ExecuteRenderCommandList(list);
}

PostProcessingPass::~PostProcessingPass()
{
	delete data;
}
