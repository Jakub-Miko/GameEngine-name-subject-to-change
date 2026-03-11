#include "DepthPrepass.h"
#include <Renderer/RootSignature.h>
#include <Renderer/MaterialManager.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/Renderer3D/RenderResourceCollection.h>
#include <Renderer/MeshManager.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <Application.h>

struct GeometryPassPreset;

template<>
struct VertexLayoutFactory<GeometryPassPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "normal"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv")
				});


			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

struct DepthPrepass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<Pipeline> pipeline_skeletal;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;
	std::shared_ptr<RenderFrameBufferResource> output_buffer;

	bool initialized = false;
};

struct PushConstantGeometryPassData {
	glm::mat4 mv;
};

void DepthPrepass::InitPostProcessingPassData() {
	GraphicsPipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.layout = VertexLayoutFactory<GeometryPassPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.framebuffer_format.color_attachemt_formats = {};
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/DepthPrepassShader.glsl");
	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.layout = VertexLayoutFactory<SkeletalGeometryPassPreset>::GetLayout();
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/SkeletalDepthPrepassShader.glsl");
	data->pipeline_skeletal = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	RenderBufferDescriptor const_desc(sizeof(glm::mat4), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);

	data->initialized = true;
}

void DepthPrepass::RenderGeometry(std::shared_ptr<RenderCommandList> list, render_props& props) {
	PROFILE("DeferredGeometryPass");
	auto& geometry = props.resource_manager.GetResource<RenderResourceCollection<Entity>>(input_geometry);
	auto& world = Application::GetWorld();

	list->SetPipeline(data->pipeline);
	list->SetRenderTarget(data->output_buffer);
	list->SetConstantBuffer("mvp", data->constant_scene_buf);

	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<MeshComponent>(entity);
		world.UpdateMesh(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);

		list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
		list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());
		glm::mat4 mv_matrix = props.view_matrix * transform.TransformMatrix;

		PushConstantGeometryPassData push_data = {};
		push_data.mv = mv_matrix;

		list->SetPushConstantRange(&push_data, sizeof(PushConstantGeometryPassData));
		list->Draw(mesh.GetMesh()->GetIndexCount());
	}
}

void DepthPrepass::RenderSkeletalGeometry(std::shared_ptr<RenderCommandList> list, render_props& props) {
	PROFILE("DeferredSkeletalGeometryPass");
	auto& geometry = props.resource_manager.GetResource<RenderResourceCollection<Entity>>(input_skeletal_geometry);
	auto& world = Application::GetWorld();
	list->SetPipeline(data->pipeline_skeletal);
	list->SetRenderTarget(data->output_buffer);
	list->SetConstantBuffer("mvp", data->constant_scene_buf);

	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
		world.UpdateSkeletalMesh(entity);
		mesh.GetAnimation().UpdateAnimation(Application::GetDeltaTime(), list, mesh.GetMesh());
	}

	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);


		list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
		list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());

		list->SetConstantBuffer("bones", mesh.GetAnimation().GetBoneBuffer());

		glm::mat4 mv_matrix = props.view_matrix * transform.TransformMatrix;

		PushConstantGeometryPassData push_data = {};
		push_data.mv = mv_matrix;
		list->SetPushConstantRange(&push_data, sizeof(PushConstantGeometryPassData));
		list->Draw(mesh.GetMesh()->GetIndexCount());
	}
}

void DepthPrepass::UpdatePrepassFrameBuffer(std::shared_ptr<RenderFrameBufferResource> g_buffer) {
	auto depth_texture = g_buffer->GetBufferDescriptor().GetDepthAttachmentAsTexture();
	if(data->output_buffer && data->output_buffer->GetBufferDescriptor().GetDepthAttachmentAsTexture() == depth_texture) {
		return;
	}

	RenderFrameBufferDescriptor framebuffer_desc = {};
	framebuffer_desc.color_attachments = {};
	framebuffer_desc.depth_stencil_attachment = { 0,depth_texture };

	data->output_buffer = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);
}


DepthPrepass::DepthPrepass(const std::string& input_geometry,const std::string& input_skeletal_geometry, const std::string& input_buffer,
                           const std::string& output_buffer_after_prepass) : input_geometry(input_geometry), output_buffer_after_prepass(output_buffer_after_prepass), input_buffer(input_buffer),
                                                                    input_skeletal_geometry(input_skeletal_geometry)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void DepthPrepass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_skeletal_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer_after_prepass, RenderPassResourceDescriptor_Access::WRITE);

	enable_depth_prepass_prop = setup_builder.GetProperties()->SetProperty("Enable Depth Prepass", true).second;

}

void DepthPrepass::Render(RenderPipelineResourceManager& resource_manager)
{
	PROFILE("DepthPrepass");
	if(!enable_depth_prepass_prop->GetValueTyped()) {
		resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer_after_prepass, resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer));
		return;
	}
	auto list = Renderer::Get()->GetRenderCommandList();
	auto& world = Application::GetWorld();
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	camera.UpdateProjectionMatrix();

	render_props props(resource_manager);
	props.gbuffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer);
	props.projection_matrix = camera.GetProjectionMatrix();
	props.view_matrix = glm::inverse(camera_trans.TransformMatrix);

	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(props.projection_matrix), sizeof(glm::mat4), 0);

	UpdatePrepassFrameBuffer(props.gbuffer);

	RenderGeometry(list, props);
	RenderSkeletalGeometry(list, props);

	Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer_after_prepass, props.gbuffer);
}

DepthPrepass::~DepthPrepass()
{
	if (data) {
		delete data;
	}
}