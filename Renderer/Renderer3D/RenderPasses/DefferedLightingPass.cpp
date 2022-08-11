#include "DefferedLightingPass.h"
#include <Renderer/RootSignature.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/Renderer3D/RenderResourceCollection.h>
#include <Renderer/MeshManager.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/LightComponent.h>
#include <Application.h>
#include <Window.h>

struct LightingPassPreset;

template<>
struct VertexLayoutFactory<LightingPassPreset> {

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


struct DefferedLightingPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<RenderFrameBufferResource> output_buffer_resource;
	std::shared_ptr<Mesh> sphere_mesh;
	std::shared_ptr<Material> mat;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;
	bool initialized = false;
};

void DefferedLightingPass::InitPostProcessingPassData() {
	PipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_BLEND;
	pipeline_desc.cull_mode = CullMode::FRONT;
	pipeline_desc.blend_equation = BlendEquation::ADD;
	pipeline_desc.layout = VertexLayoutFactory<LightingPassPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("LightingPassShader.glsl");
	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	TextureSamplerDescritor sampler_desc;
	sampler_desc.AddressMode_U = TextureAddressMode::BORDER;
	sampler_desc.AddressMode_V = TextureAddressMode::BORDER;
	sampler_desc.AddressMode_W = TextureAddressMode::BORDER;
	sampler_desc.border_color = glm::vec4(1.0, 0.4, 1.0, 1.0);
	sampler_desc.filter = TextureFilter::POINT_MIN_MAG_MIP;
	sampler_desc.LOD_bias = 0;
	sampler_desc.min_LOD = 0;
	sampler_desc.max_LOD = 10;

	auto sampler = TextureSampler::CreateSampler(sampler_desc);

	RenderTexture2DDescriptor color_texture_desc;
	color_texture_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
	color_texture_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	color_texture_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	color_texture_desc.sampler = sampler;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
	depth_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	depth_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	depth_desc.sampler = sampler;

	auto texture_color = RenderResourceManager::Get()->CreateTexture(color_texture_desc);
	auto texture_depth_stencil = RenderResourceManager::Get()->CreateTexture(depth_desc);

	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = { texture_color };
	framebuffer_desc.depth_stencil_attachment = texture_depth_stencil;

	data->output_buffer_resource = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);

	RenderBufferDescriptor const_desc(sizeof(glm::mat4) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);

	data->sphere_mesh = MeshManager::Get()->LoadMeshFromFileAsync("asset:Sphere.mesh"_path);
	data->mat = MaterialManager::Get()->CreateMaterial("LightingPassShader.glsl");

	data->initialized = true;
}


DefferedLightingPass::DefferedLightingPass(const std::string& input_gbuffer, const std::string& input_lights, const std::string& output_buffer)
	: input_gbuffer(input_gbuffer), output_buffer(output_buffer), input_lights(input_lights)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void DefferedLightingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer, RenderPassResourceDescriptor_Access::READ);
}

void DefferedLightingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	auto& geometry = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_lights);
	auto& gbuffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer);
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	auto& world = Application::GetWorld();
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	auto& camera_transform = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	camera.UpdateProjectionMatrix();
	auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	auto ViewProjection = camera.GetProjectionMatrix() * glm::inverse(camera_trans.TransformMatrix);
	list->SetPipeline(data->pipeline);
	list->SetRenderTarget(data->output_buffer_resource);
	list->Clear();
	list->SetConstantBuffer("mvp", data->constant_scene_buf);
	for (auto& entity : geometry.resources) {
		auto transform = world.GetComponent<TransformComponent>(entity).TransformMatrix;
		auto& light = world.GetComponent<LightComponent>(entity);

		if (light.type == LightType::DIRECTIONAL) {
			glm::scale(transform, glm::vec3(10000.0f));
		}

		glm::vec2 pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
			1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };

		data->mat->SetParameter("pixel_size", pixel_size);
		data->mat->SetParameter("Light_Color", light.color);
		data->mat->SetParameter("Color", gbuffer->GetBufferDescriptor().color_attachments[0]);
		data->mat->SetParameter("World_Position", gbuffer->GetBufferDescriptor().color_attachments[1]);
		data->mat->SetParameter("Normal", gbuffer->GetBufferDescriptor().color_attachments[2]);
		data->mat->SetMaterial(list, data->pipeline);
		list->SetVertexBuffer(data->sphere_mesh->GetVertexBuffer());
		list->SetIndexBuffer(data->sphere_mesh->GetIndexBuffer());
		glm::mat4 mvp = ViewProjection * transform;
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mvp), sizeof(glm::mat4), 0);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(transform), sizeof(glm::mat4), sizeof(glm::mat4));
		list->Draw(data->sphere_mesh->GetIndexCount());


	}

	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, data->output_buffer_resource);


}

DefferedLightingPass::~DefferedLightingPass()
{
	if (data) {
		delete data;
	}
}