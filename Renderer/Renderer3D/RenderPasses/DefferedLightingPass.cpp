#include "DefferedLightingPass.h"
#include <Renderer/TextureManager.h>
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
#include <World/Components/ShadowCasterComponent.h>
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
	std::shared_ptr<Pipeline> pipeline_shadowed;
	std::shared_ptr<RenderFrameBufferResource> output_buffer_resource;
	std::shared_ptr<Mesh> sphere_mesh;
	std::shared_ptr<Mesh> card_mesh;
	std::shared_ptr<Material> mat;
	std::shared_ptr<Material> mat_shadowed;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;
	std::shared_ptr<RenderBufferResource> constant_scene_buf_shadowed;
	bool initialized = false;
};

void DefferedLightingPass::InitPostProcessingPassData() {
	PipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	PipelineBlendFunctions blend_function;
	blend_function.dstAlpha = BlendFunction::ONE;
	blend_function.srcAlpha = BlendFunction::ONE;
	blend_function.srcRGB = BlendFunction::ONE;
	blend_function.dstRGB = BlendFunction::ONE;
	pipeline_desc.blend_functions = blend_function;
	pipeline_desc.flags = PipelineFlags::ENABLE_BLEND;
	pipeline_desc.cull_mode = CullMode::FRONT;
	pipeline_desc.blend_equation = BlendEquation::ADD;
	pipeline_desc.layout = VertexLayoutFactory<LightingPassPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/LightingPassShader.glsl");
	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/LightingPassShaderShadowed.glsl");
	data->pipeline_shadowed = PipelineManager::Get()->CreatePipeline(pipeline_desc);

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

	RenderBufferDescriptor const_desc(sizeof(glm::mat4) * 3 + sizeof(float) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);

	RenderBufferDescriptor const_desc_shadowed(sizeof(glm::mat4) * 4 + sizeof(float) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf_shadowed = RenderResourceManager::Get()->CreateBuffer(const_desc_shadowed);

	data->sphere_mesh = MeshManager::Get()->LoadMeshFromFileAsync("asset:Sphere.mesh"_path);
	data->mat = MaterialManager::Get()->CreateMaterial("shaders/LightingPassShader.glsl");
	data->mat_shadowed = MaterialManager::Get()->CreateMaterial("shaders/LightingPassShaderShadowed.glsl");

	struct Vertex {
		Vertex(glm::vec3 pos, glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 uv = glm::vec3(0.0f))
			: pos(pos), normal(normal), tangent(tangent), uv(uv) {}
		glm::vec3 pos;
		glm::vec3 normal = glm::vec3(0.0f,0.0f,1.0f);
		glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec2 uv = glm::vec3(0.0f);
	};

	Vertex card_vertecies[4] = {
		Vertex({-1,-1,0}),
		Vertex({-1,1,0}),
		Vertex({1,-1,0}),
		Vertex({1,1,0})
	};
	
	unsigned int indicies[6] = { 0,1,2,1,3,2 };

	RenderBufferDescriptor index_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
	RenderBufferDescriptor vertex_desc(sizeof(card_vertecies), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
	auto card_vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_desc);
	auto card_index_buffer = RenderResourceManager::Get()->CreateBuffer(index_desc);
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();

	RenderResourceManager::Get()->ReallocateAndUploadBuffer(list, card_vertex_buffer, (void*)card_vertecies, sizeof(card_vertecies));
	RenderResourceManager::Get()->ReallocateAndUploadBuffer(list, card_index_buffer, (void*)indicies, sizeof(indicies));

	queue->ExecuteRenderCommandList(list);

	data->card_mesh = std::shared_ptr<Mesh>(new Mesh(card_vertex_buffer, card_index_buffer, 6));

	data->initialized = true;
}


DefferedLightingPass::DefferedLightingPass(const std::string& input_gbuffer, const std::string& input_lights, const std::string& input_directional_shadowed_lights, const std::string& input_point_shadowed_lights, const std::string& output_buffer, const std::string& shadow_map_dependency_tag)
	: input_gbuffer(input_gbuffer), output_buffer(output_buffer), input_lights(input_lights), input_directional_shadowed_lights(input_directional_shadowed_lights),
	input_point_shadowed_lights(input_point_shadowed_lights), shadow_map_dependency_tag(shadow_map_dependency_tag)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void DefferedLightingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_directional_shadowed_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_point_shadowed_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<DependencyTag>(shadow_map_dependency_tag, RenderPassResourceDescriptor_Access::READ);
}

void DefferedLightingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	render_props props;
	auto& gbuffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer);
	auto& world = Application::GetWorld();
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	camera.UpdateProjectionMatrix();
	auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	props.projection = camera.GetProjectionMatrix();
	props.view = glm::inverse(camera_trans.TransformMatrix);

	props.depth_constant_a = camera.zFar / (camera.zFar - camera.zNear);
	props.depth_constant_b = (-camera.zFar * camera.zNear) / (camera.zFar - camera.zNear);
	
	list->SetRenderTarget(data->output_buffer_resource);
	list->Clear();

	RenderResourceManager::Get()->CopyFrameBufferDepthAttachment(list, gbuffer, data->output_buffer_resource);
	RenderLights(resource_manager, list, camera, props);
	RenderShadowedLights<LightType::DIRECTIONAL>(resource_manager, list, camera, props);
	RenderShadowedLights<LightType::POINT>(resource_manager, list, camera, props);


	queue->ExecuteRenderCommandList(list);



	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, data->output_buffer_resource);


}

DefferedLightingPass::~DefferedLightingPass()
{
	if (data) {
		delete data;
	}
}

void DefferedLightingPass::RenderLights(RenderPipelineResourceManager& resource_manager,RenderCommandList* list, const CameraComponent& camera,const render_props& props)
{
	auto& geometry = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_lights);
	auto& gbuffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer);
	auto& world = Application::GetWorld();
	auto ViewProjection = props.projection * props.view;
	auto view_matrix = props.view;
	list->SetPipeline(data->pipeline);
	list->SetRenderTarget(data->output_buffer_resource);
	list->SetConstantBuffer("conf", data->constant_scene_buf);
	float depth_constant_a = props.depth_constant_a;
	float depth_constant_b = props.depth_constant_b;
	glm::mat4 inverse_projection = glm::inverse(props.projection);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, &depth_constant_a, sizeof(float), sizeof(glm::mat4) * 3);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, &depth_constant_b, sizeof(float), sizeof(glm::mat4) * 3 + sizeof(float));
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, &inverse_projection, sizeof(glm::mat4), sizeof(glm::mat4) * 2);

	for (auto& entity : geometry.resources) {
		auto& transform_component = world.GetComponent<TransformComponent>(entity);
		auto transform = transform_component.TransformMatrix;
		auto& light = world.GetComponent<LightComponent>(entity);
		size_t index_count = 0;
		glm::mat4 mvp;
		glm::mat4 mv_matrix;

		if (light.type == LightType::DIRECTIONAL) {
			mvp = glm::mat4(1.0f);
			mv_matrix = view_matrix * transform;
			list->SetVertexBuffer(data->card_mesh->GetVertexBuffer());
			list->SetIndexBuffer(data->card_mesh->GetIndexBuffer());
			index_count = data->card_mesh->GetIndexCount();
		}
		else if (light.type == LightType::POINT) {
			glm::mat4 model_sphere = glm::translate(glm::mat4(1.0f), (glm::vec3)transform_component.TransformMatrix[3]) * glm::scale(glm::mat4(1.0), glm::vec3(light.CalcRadiusFromAttenuation()));
			mv_matrix = view_matrix * model_sphere;
			mvp = ViewProjection * model_sphere;
			list->SetVertexBuffer(data->sphere_mesh->GetVertexBuffer());
			list->SetIndexBuffer(data->sphere_mesh->GetIndexBuffer());
			index_count = data->sphere_mesh->GetIndexCount();
		}

		glm::vec2 pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
			1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };

		data->mat->SetParameter("pixel_size", pixel_size);
		data->mat->SetParameter("Light_Color", light.GetLightColor());
		data->mat->SetParameter("Color", gbuffer->GetBufferDescriptor().GetColorAttachmentAsTexture(0));
		data->mat->SetParameter("Normal", gbuffer->GetBufferDescriptor().GetColorAttachmentAsTexture(1));
		data->mat->SetParameter("DepthBuffer", gbuffer->GetBufferDescriptor().GetDepthAttachmentAsTexture());
		data->mat->SetParameter("light_type", (int)light.type);
		data->mat->SetParameter("attenuation", glm::vec4(light.GetAttenuation(), 0.0f));
		data->mat->SetMaterial(list, data->pipeline);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mvp), sizeof(glm::mat4), 0);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mv_matrix), sizeof(glm::mat4), sizeof(glm::mat4));
		list->Draw(index_count);


	}
}

template<LightType light_type>
void DefferedLightingPass::RenderShadowedLights(RenderPipelineResourceManager& resource_manager, RenderCommandList* list, const CameraComponent& camera, const render_props& props)
{
	const RenderResourceCollection<Entity>* geometry;
	if constexpr (light_type == LightType::DIRECTIONAL) {
		geometry = &resource_manager.GetResource<RenderResourceCollection<Entity>>(input_directional_shadowed_lights);
		data->mat_shadowed->SetParameter("ShadowMap", TextureManager::Get()->GetDefaultTexture());
	}else if constexpr (light_type == LightType::POINT) {
		geometry = &resource_manager.GetResource<RenderResourceCollection<Entity>>(input_point_shadowed_lights);
		data->mat_shadowed->SetParameter("ShadowCubeMap", TextureManager::Get()->GetDefaultTextureCubemap());
	}
	auto& gbuffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer);
	auto& world = Application::GetWorld();
	auto ViewProjection = props.projection * props.view;
	auto view_matrix = props.view;
	list->SetPipeline(data->pipeline_shadowed);
	list->SetRenderTarget(data->output_buffer_resource);
	list->SetConstantBuffer("conf", data->constant_scene_buf_shadowed);
	float depth_constant_a = props.depth_constant_a;
	float depth_constant_b = props.depth_constant_b;
	glm::mat4 inverse_projection = glm::inverse(props.projection);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, &depth_constant_a, sizeof(float), sizeof(glm::mat4) * 4);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, &depth_constant_b, sizeof(float), sizeof(glm::mat4) * 4 + sizeof(float));
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, &inverse_projection, sizeof(glm::mat4), sizeof(glm::mat4) * 3);
	for (auto& entity : geometry->resources) {
		auto& transform_component = world.GetComponent<TransformComponent>(entity);
		auto transform = transform_component.TransformMatrix;
		auto& light = world.GetComponent<LightComponent>(entity);
		auto& shadow = world.GetComponent<ShadowCasterComponent>(entity);
		size_t index_count = 0;
		glm::mat4 mvp;
		glm::mat4 mv_matrix;

		if constexpr(light_type == LightType::DIRECTIONAL) {
			mvp = glm::mat4(1.0f);
			mv_matrix = view_matrix * transform;
			list->SetVertexBuffer(data->card_mesh->GetVertexBuffer());
			list->SetIndexBuffer(data->card_mesh->GetIndexBuffer());
			data->mat_shadowed->SetParameter("ShadowMap", shadow.shadow_map->GetBufferDescriptor().GetDepthAttachmentAsTexture());
			index_count = data->card_mesh->GetIndexCount();
		}
		else if constexpr (light_type == LightType::POINT) {
			glm::mat4 model_sphere = glm::translate(glm::mat4(1.0f), (glm::vec3)transform_component.TransformMatrix[3]) * glm::scale(glm::mat4(1.0), glm::vec3(light.CalcRadiusFromAttenuation()));
			mv_matrix = view_matrix * model_sphere;
			mvp = ViewProjection * model_sphere;
			list->SetVertexBuffer(data->sphere_mesh->GetVertexBuffer());
			list->SetIndexBuffer(data->sphere_mesh->GetIndexBuffer());
			data->mat_shadowed->SetParameter("light_far_plane", shadow.far_plane);
			data->mat_shadowed->SetParameter("ShadowCubeMap", shadow.shadow_map->GetBufferDescriptor().GetDepthAttachmentAsTextureCubemap());
			index_count = data->sphere_mesh->GetIndexCount();
		}

		auto inverse_view = Application::GetWorld().GetComponent<TransformComponent>(Application::GetWorld().GetPrimaryEntity()).TransformMatrix;

		glm::mat4 light_matrix = shadow.light_view_matrix * inverse_view;
		glm::vec2 pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
			1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };

		data->mat_shadowed->SetParameter("pixel_size", pixel_size);
		data->mat_shadowed->SetParameter("Light_Color", light.GetLightColor());
		data->mat_shadowed->SetParameter("Color", gbuffer->GetBufferDescriptor().GetColorAttachmentAsTexture(0));
		data->mat_shadowed->SetParameter("Normal", gbuffer->GetBufferDescriptor().GetColorAttachmentAsTexture(1));
		data->mat_shadowed->SetParameter("DepthBuffer", gbuffer->GetBufferDescriptor().GetDepthAttachmentAsTexture());
		data->mat_shadowed->SetParameter("light_type", (int)light.type);
		data->mat_shadowed->SetParameter("attenuation", glm::vec4(light.GetAttenuation(), 0.0f));
		data->mat_shadowed->SetMaterial(list, data->pipeline_shadowed);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, glm::value_ptr(mvp), sizeof(glm::mat4), 0);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, glm::value_ptr(mv_matrix), sizeof(glm::mat4), sizeof(glm::mat4));
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_shadowed, glm::value_ptr(light_matrix), sizeof(glm::mat4), sizeof(glm::mat4) * 2);
		list->Draw(index_count);


	}
}