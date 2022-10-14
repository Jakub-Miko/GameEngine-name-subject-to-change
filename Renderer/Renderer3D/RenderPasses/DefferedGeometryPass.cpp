#include "DefferedGeometryPass.h"
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
#include <Application.h>
#include <Window.h>

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


struct DefferedGeometryPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<RenderFrameBufferResource> output_buffer_resource;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;
	bool initialized = false;
};

void DefferedGeometryPass::InitPostProcessingPassData() {
	PipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.layout = VertexLayoutFactory<GeometryPassPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/GeometryPassShader.glsl");
	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	TextureSamplerDescritor sampler_desc;
	sampler_desc.AddressMode_U = TextureAddressMode::BORDER;
	sampler_desc.AddressMode_V = TextureAddressMode::BORDER;
	sampler_desc.AddressMode_W = TextureAddressMode::BORDER;
	sampler_desc.border_color = glm::vec4(1.0, 0.4, 1.0, 1.0);
	sampler_desc.filter = TextureFilter::POINT_MIN_MAG;
	sampler_desc.LOD_bias = 0;
	sampler_desc.min_LOD = 0;
	sampler_desc.max_LOD = 10;
	
	auto sampler = TextureSampler::CreateSampler(sampler_desc);

	RenderTexture2DDescriptor color_texture_desc;
	color_texture_desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
	color_texture_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	color_texture_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	color_texture_desc.sampler = sampler;

	RenderTexture2DDescriptor color_normal_desc;
	color_normal_desc.format = TextureFormat::RGBA_32FLOAT;
	color_normal_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	color_normal_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	color_normal_desc.sampler = sampler;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
	depth_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	depth_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	depth_desc.sampler = sampler;

	auto texture_color_albedo = RenderResourceManager::Get()->CreateTexture(color_texture_desc);
	auto texture_color_normal = RenderResourceManager::Get()->CreateTexture(color_normal_desc);
	auto texture_depth_stencil = RenderResourceManager::Get()->CreateTexture(depth_desc);

	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = { texture_color_albedo,texture_color_normal };
	framebuffer_desc.depth_stencil_attachment = texture_depth_stencil;
	
	data->output_buffer_resource = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);

	RenderBufferDescriptor const_desc(sizeof(glm::mat4)*2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);


	data->initialized = true;
}


DefferedGeometryPass::DefferedGeometryPass(const std::string& input_geometry, const std::string& output_buffer) : input_geometry(input_geometry), output_buffer(output_buffer)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void DefferedGeometryPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
}

void DefferedGeometryPass::Render(RenderPipelineResourceManager& resource_manager)
{
	auto& geometry = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_geometry);
	auto queue = Renderer::Get()->GetCommandQueue(); 
	auto list = Renderer::Get()->GetRenderCommandList();
	auto& world = Application::GetWorld();
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	auto& camera_transform = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	camera.UpdateProjectionMatrix();
	auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	auto ViewProjection = camera.GetProjectionMatrix() * glm::inverse(camera_trans.TransformMatrix);
	auto view_matrix = glm::inverse(camera_trans.TransformMatrix);
	list->SetPipeline(data->pipeline);
	list->SetRenderTarget(data->output_buffer_resource);
	list->Clear();
	list->SetConstantBuffer("mvp", data->constant_scene_buf);
	auto default_mat = data->pipeline->GetShader()->GetDefaultMaterial();
	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<MeshComponent>(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);

		if (mesh.material == nullptr) {
			default_mat->SetMaterial(list, data->pipeline);
		}
		else {
			mesh.material->SetMaterial(list, data->pipeline);
		}
		list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
		list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());
		glm::mat4 mvp = ViewProjection * transform.TransformMatrix;
		glm::mat4 mv_matrix = view_matrix * transform.TransformMatrix;
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mvp), sizeof(glm::mat4), 0);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mv_matrix), sizeof(glm::mat4), sizeof(glm::mat4));
		list->Draw(mesh.GetMesh()->GetIndexCount());


	}

	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, data->output_buffer_resource);


}

DefferedGeometryPass::~DefferedGeometryPass()
{
	if (data) {
		delete data;
	}
}