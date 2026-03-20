#include "ClusteredLightingPass.h"
#include <Renderer/TextureManager.h>
#include <Renderer/RootSignature.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/MaterialManager.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/Renderer3D/RenderResourceCollection.h>
#include <Renderer/MeshManager.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/SkylightComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/LightComponent.h>
#include <Application.h>
#include <Window.h>
#include "ClusteredLightCullingPass.h"

struct LightingPassPreset;

#define COMPUTE_TILE_SIZE 16

template<>
struct VertexLayoutFactory<LightingPassPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "position"),
				});

			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

struct LightData {
	glm::mat4 view_model_matrix;
	glm::vec4 light_color;
	float range;
	int light_type;
	uint8_t padding[8]; // pad to match 16 byte alignment requirements of vec4 and mat4
};

struct ConfigData {
	glm::mat4 projection_matrix;
	glm::mat4 inverse_view_matrix;
	glm::vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	glm::uvec3 cluster_grid_size;
	int point_light_count;
	int directional_light_count;
	int skylight_count;
	float near_plane;
	float far_plane;
};

struct ClusteredLightingPass::internal_data {
	std::shared_ptr<Pipeline> pipeline_clustered;
	std::shared_ptr<Pipeline> pipeline_skylight;
	std::shared_ptr<Pipeline> pipeline_shadowed_directional;
	std::shared_ptr<Pipeline> pipeline_bg;
	std::shared_ptr<RenderFrameBufferResource> output_buffer_resource;
	std::shared_ptr<Mesh> card_mesh;
	std::shared_ptr<Material> mat_skylight;
	std::shared_ptr<Material> mat_shadowed_directional;
	std::shared_ptr<RenderBufferResource> light_list;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;
	std::shared_ptr<RenderBufferResource> constant_scene_buf_skylight;
	std::shared_ptr<RenderBufferResource> constant_scene_buf_shadowed_directional;
	std::shared_ptr<RenderBufferResource> constant_scene_buf_bg;
	std::shared_ptr<RenderTexture2DResource> color_storage_texture;
	bool initialized = false;
};

void ClusteredLightingPass::InitPassData() {
	UpdateClusteredPipeline(true);

	GraphicsPipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	PipelineBlendFunctions blend_function;
	blend_function.dstAlpha = BlendFunction::ONE;
	blend_function.srcAlpha = BlendFunction::ONE;
	blend_function.srcRGB = BlendFunction::ONE;
	blend_function.dstRGB = BlendFunction::ONE;
	pipeline_desc.blend_functions = blend_function;
	pipeline_desc.enable_depth_clip = false;
	pipeline_desc.flags = PipelineFlags::ENABLE_BLEND;
	pipeline_desc.cull_mode = CullMode::FRONT;
	pipeline_desc.blend_equation = BlendEquation::ADD;
	pipeline_desc.layout = VertexLayoutFactory<LightingPassPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/LightingPassShaderSkylight.glsl");
	pipeline_desc.framebuffer_format.color_attachemt_formats = {
		{ TextureFormat::RGBA_16FLOAT }
	};

	data->pipeline_skylight = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/LightingPassShaderShadowedDirectional.glsl");
	data->pipeline_shadowed_directional = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.depth_function = DepthFunction::LESS_EQUAL;
	pipeline_desc.enable_depth_clip = false;
	pipeline_desc.layout = VertexLayoutFactory<LightingPassPreset>::GetLayout();
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/CubeMapRender.glsl");
	data->pipeline_bg = PipelineManager::Get()->CreatePipeline(pipeline_desc);

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
	color_texture_desc.format = TextureFormat::RGBA_16FLOAT;
	color_texture_desc.usage = TextureUsage::COLOR_ATTACHMENT_READABLE;
	color_texture_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	color_texture_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	color_texture_desc.sampler = sampler;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEFAULT_DEPTH;
	depth_desc.usage = TextureUsage::DEPTH_ATTACHMENT_READABLE;
	depth_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	depth_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	depth_desc.sampler = sampler;

	auto texture_color = RenderResourceManager::Get()->CreateTexture(color_texture_desc);
	auto texture_depth_stencil = RenderResourceManager::Get()->CreateTexture(depth_desc);

	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = { {0,texture_color} };
	framebuffer_desc.depth_stencil_attachment = { 0,texture_depth_stencil };

	data->output_buffer_resource = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);

	RenderTexture2DDescriptor color_storage_texture_desc = {};
	color_storage_texture_desc.format = TextureFormat::RGBA_16FLOAT;
	color_storage_texture_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	color_storage_texture_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	color_storage_texture_desc.usage = TextureUsage::STORAGE_READABLE;
	color_storage_texture_desc.sampler = sampler;
	data->color_storage_texture = RenderResourceManager::Get()->CreateTexture(color_storage_texture_desc);

	RenderBufferDescriptor const_desc(sizeof(glm::mat4) * 3 + sizeof(float) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);

	RenderBufferDescriptor const_desc_shadowed_directional(sizeof(glm::mat4) * 18 + sizeof(float) * 5 + sizeof(glm::vec2) + sizeof(uint32_t), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf_shadowed_directional = RenderResourceManager::Get()->CreateBuffer(const_desc_shadowed_directional);

	RenderBufferDescriptor const_desc_skylight(sizeof(glm::mat4) * 2 + sizeof(float) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf_skylight = RenderResourceManager::Get()->CreateBuffer(const_desc_skylight);

	RenderBufferDescriptor const_desc_bg(sizeof(glm::mat4) + sizeof(glm::vec4), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_scene_buf_bg = RenderResourceManager::Get()->CreateBuffer(const_desc_bg);

	data->mat_shadowed_directional = MaterialManager::Get()->CreateMaterial("LightingPassDirectionalLightMaterial");
	data->mat_skylight = MaterialManager::Get()->CreateMaterial("LightingPassSkylightLightProps");

	RenderBufferDescriptor light_list_desc(sizeof(LightData) * 100, RenderBufferType::DEFAULT, RenderBufferUsage::STORAGE_BUFFER);
	data->light_list = RenderResourceManager::Get()->CreateBuffer(light_list_desc);

	struct Vertex {
		Vertex(glm::vec2 pos)
			: pos(pos) {}
		glm::vec2 pos;
	};

	Vertex card_vertecies[4] = {
		Vertex({-1,-1}),
		Vertex({-1,1}),
		Vertex({1,-1}),
		Vertex({1,1})
	};
	
	unsigned int indicies[6] = { 0,1,2,1,3,2 };

	RenderBufferDescriptor index_desc(sizeof(indicies), RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
	RenderBufferDescriptor vertex_desc(sizeof(card_vertecies), RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
	auto card_vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_desc);
	auto card_index_buffer = RenderResourceManager::Get()->CreateBuffer(index_desc);
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();

	RenderResourceManager::Get()->UploadDataToBuffer(list, card_vertex_buffer, (void*)card_vertecies, sizeof(card_vertecies),0);
	RenderResourceManager::Get()->UploadDataToBuffer(list, card_index_buffer, (void*)indicies, sizeof(indicies),0);

	queue->ExecuteRenderCommandList(list);

	data->card_mesh = std::shared_ptr<Mesh>(new Mesh(card_vertex_buffer, card_index_buffer, 6));

	data->initialized = true;
}


ClusteredLightingPass::ClusteredLightingPass(const std::string& input_gbuffer, const std::string& input_gbuffer_material, const std::string& input_clustered_lights, const std::string& input_directional_shadowed_lights,
	const std::string& input_point_shadowed_lights, const std::string& output_texture, const std::string& shadow_map_dependency_tag,
	const std::string& input_point_shadow_maps, const std::string& input_directional_shadow_maps)
	: input_gbuffer(input_gbuffer), output_texture(output_texture), input_clustered_lights(input_clustered_lights), input_directional_shadowed_lights(input_directional_shadowed_lights),
	input_point_shadowed_lights(input_point_shadowed_lights), shadow_map_dependency_tag(shadow_map_dependency_tag),
	input_gbuffer_material(input_gbuffer_material), input_directional_shadow_maps(input_directional_shadow_maps), input_point_shadow_maps(input_point_shadow_maps)
{
	data = new internal_data;
}

void ClusteredLightingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<ClusteredLightLists>(input_clustered_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_directional_shadowed_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_point_shadowed_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<Material>>(input_gbuffer_material, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderTexture2DResource>>(output_texture, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_gbuffer, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<DependencyTag>(shadow_map_dependency_tag, RenderPassResourceDescriptor_Access::READ);

	clustered_config.use_compute_for_clustered_lights = setup_builder.GetProperties()->SetProperty("Use compute shader for clustered lights", false).second;
	clustered_config.scalarize_lights = setup_builder.GetProperties()->SetProperty("Scalarize lights", true).second;
	clustered_config.compute_tile_size = setup_builder.GetProperties()->SetProperty<uint32_t>("Compute tile size", 16).second;
	clustered_config.needs_update = setup_builder.GetProperties()->SetProperty("Update clustered shading settings", DynamicPropertyAction()).second;

	InitPassData();
}

void ClusteredLightingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	PROFILE("ClusteredLightingPass");
	render_props props;
	UpdateClusteredPipeline();
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

	auto dynamic_props = resource_manager.GetProperties();


	RenderResourceManager::Get()->CopyFrameBufferDepthAttachment(list, gbuffer, data->output_buffer_resource);

	if(use_compute_for_clustered_lights) {
		RenderLightsWithCompute(resource_manager, list, camera, props);
	} else {
		RenderLights(resource_manager, list, camera, props);
	}

	RenderSkybox(resource_manager, list, camera, props);

	queue->ExecuteRenderCommandList(list);


	if(use_compute_for_clustered_lights) {
		resource_manager.SetResource<std::shared_ptr<RenderTexture2DResource>>(output_texture, data->color_storage_texture);
	} else {
		resource_manager.SetResource<std::shared_ptr<RenderTexture2DResource>>(output_texture, data->output_buffer_resource->GetBufferDescriptor().GetColorAttachmentAsTexture(0));
	}
}

ClusteredLightingPass::~ClusteredLightingPass()
{
	if (data) {
		delete data;
	}
}

void ClusteredLightingPass::UpdateClusteredPipeline(bool force_update) {
	auto& needs_update = clustered_config.needs_update->GetValueTyped();
	if(!needs_update.ShouldActivate() && !force_update) {
		return;
	}
	needs_update.Reset();

	if(clustered_config.use_compute_for_clustered_lights->GetValueTyped()) {
		std::vector<std::string> shader_defines;
		shader_defines.push_back("COMPUTE_TILE_SIZE=" + std::to_string(clustered_config.compute_tile_size->GetValueTyped()));
		compute_tile_size = clustered_config.compute_tile_size->GetValueTyped();
		if(clustered_config.scalarize_lights->GetValueTyped()) shader_defines.push_back("SCALARIZE");

		ComputePipelineDescriptor compute_pipeline_desc = {};
		compute_pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredLightingPassShaderCompute.glsl", shader_defines);
		data->pipeline_clustered = PipelineManager::Get()->CreatePipeline(compute_pipeline_desc);
		use_compute_for_clustered_lights = true;
	} else {
		GraphicsPipelineDescriptor pipeline_desc;
		pipeline_desc.viewport = RenderViewport();
		pipeline_desc.scissor_rect = RenderScissorRect();
		PipelineBlendFunctions blend_function;
		blend_function.dstAlpha = BlendFunction::ONE;
		blend_function.srcAlpha = BlendFunction::ONE;
		blend_function.srcRGB = BlendFunction::ONE;
		blend_function.dstRGB = BlendFunction::ONE;
		pipeline_desc.blend_functions = blend_function;
		pipeline_desc.enable_depth_clip = false;
		pipeline_desc.flags = PipelineFlags::ENABLE_BLEND;
		pipeline_desc.cull_mode = CullMode::FRONT;
		pipeline_desc.blend_equation = BlendEquation::ADD;
		pipeline_desc.layout = VertexLayoutFactory<LightingPassPreset>::GetLayout();
		pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
		pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredLightingPassShader.glsl", {{"HARD_CODE_CASCADES", HARD_CODE_CASCADES}});
		pipeline_desc.framebuffer_format.color_attachemt_formats = {
			{ TextureFormat::RGBA_16FLOAT }
		};

		data->pipeline_clustered = PipelineManager::Get()->CreatePipeline(pipeline_desc);
		use_compute_for_clustered_lights = false;
	}
}

void ClusteredLightingPass::RenderLights(RenderPipelineResourceManager& resource_manager,std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera,const render_props& props)
{
	auto& clustered_lights = resource_manager.GetResource<ClusteredLightLists>(input_clustered_lights);
	auto& point_shadow_maps = resource_manager.GetPersistentResource<std::shared_ptr<RenderResourceStore>>(input_point_shadow_maps);
	auto& directional_shadow_maps = resource_manager.GetPersistentResource<std::shared_ptr<RenderResourceStore>>(input_directional_shadow_maps);

	if(clustered_lights.num_of_point_lights == 0 && clustered_lights.num_of_directional_lights == 0) return;

	auto& gbuffer_material = resource_manager.GetResource<std::shared_ptr<Material>>(input_gbuffer_material);
	list->SetPipeline(data->pipeline_clustered);
	list->SetRenderTarget(data->output_buffer_resource);
	gbuffer_material->SetMaterial(list);
	glm::vec2 pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
		1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };

	ConfigData config_data = {};
	config_data.point_light_count = clustered_lights.num_of_point_lights;
	config_data.directional_light_count = clustered_lights.num_of_directional_lights;
	config_data.skylight_count = clustered_lights.num_of_skylights;
	config_data.cluster_grid_size = glm::uvec3(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	config_data.depth_constant_a = props.depth_constant_a;
	config_data.depth_constant_b = props.depth_constant_b;
	config_data.far_plane = camera.zFar;
	config_data.near_plane = camera.zNear;
	config_data.pixel_size = pixel_size;
	config_data.projection_matrix = props.projection;
	config_data.inverse_view_matrix = glm::inverse(props.view);

	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, &config_data, sizeof(ConfigData), 0);

	list->SetConstantBuffer("conf", data->constant_scene_buf);
	list->SetStorageBuffer("point_light_buffer", clustered_lights.point_light_buffer);
	list->SetStorageBuffer("directional_light_buffer", clustered_lights.directional_light_buffer);
	list->SetStorageBuffer("skylight_buffer", clustered_lights.skylight_buffer);
	list->SetStorageBuffer("light_assignment_buffer", clustered_lights.light_assignment_buffer);
	list->SetStorageBuffer("cluster_buffer", clustered_lights.cluster_buffer);
	list->SetVertexBuffer(data->card_mesh->GetVertexBuffer());
	list->SetIndexBuffer(data->card_mesh->GetIndexBuffer());
	list->SetResourceStore("point_light_shadow_maps", point_shadow_maps);
	list->SetResourceStore("directional_light_shadow_maps", directional_shadow_maps);
	list->SetResourceStore("skylight_reflection_maps", TextureManager::Get()->GetReflectionMapResourceStore());
	list->Draw(data->card_mesh->GetIndexCount());
}

void ClusteredLightingPass::RenderLightsWithCompute(RenderPipelineResourceManager& resource_manager,
	std::shared_ptr<RenderCommandList> list, const CameraComponent& camera, const render_props& props) {

	auto& clustered_lights = resource_manager.GetResource<ClusteredLightLists>(input_clustered_lights);
	auto& point_shadow_maps = resource_manager.GetPersistentResource<std::shared_ptr<RenderResourceStore>>(input_point_shadow_maps);
	auto& directional_shadow_maps = resource_manager.GetPersistentResource<std::shared_ptr<RenderResourceStore>>(input_directional_shadow_maps);

	if(clustered_lights.num_of_point_lights == 0 && clustered_lights.num_of_directional_lights == 0) return;

	auto& gbuffer_material = resource_manager.GetResource<std::shared_ptr<Material>>(input_gbuffer_material);
	list->SetPipeline(data->pipeline_clustered);
	list->SetStorageTexture("color_out", data->color_storage_texture);
	gbuffer_material->SetMaterial(list);
	glm::vec2 pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
		1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };

	ConfigData config_data = {};
	config_data.point_light_count = clustered_lights.num_of_point_lights;
	config_data.directional_light_count = clustered_lights.num_of_directional_lights;
	config_data.cluster_grid_size = glm::uvec3(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	config_data.depth_constant_a = props.depth_constant_a;
	config_data.depth_constant_b = props.depth_constant_b;
	config_data.far_plane = camera.zFar;
	config_data.near_plane = camera.zNear;
	config_data.pixel_size = pixel_size;
	config_data.projection_matrix = glm::inverse(props.projection);

	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, &config_data, sizeof(ConfigData), 0);

	list->SetConstantBuffer("conf", data->constant_scene_buf);
	list->SetStorageBuffer("point_light_buffer", clustered_lights.point_light_buffer);
	list->SetStorageBuffer("directional_light_buffer", clustered_lights.directional_light_buffer);
	list->SetStorageBuffer("light_assignment_buffer", clustered_lights.light_assignment_buffer);
	list->SetStorageBuffer("cluster_buffer", clustered_lights.cluster_buffer);
	list->SetResourceStore("point_light_shadow_maps", point_shadow_maps);
	list->SetResourceStore("directional_light_shadow_maps", point_shadow_maps);

	list->Dispatch(glm::ceil(data->color_storage_texture->GetBufferDescriptor().width / compute_tile_size),
		glm::ceil(data->color_storage_texture->GetBufferDescriptor().height / compute_tile_size), 1);
}

void ClusteredLightingPass::RenderSkybox(RenderPipelineResourceManager& resource_manager, std::shared_ptr<RenderCommandList>  list, const CameraComponent& camera, const render_props& props)
{
	auto skylight_view = Application::GetWorld().GetRegistry().view<SkylightComponent>();
	auto& world = Application::GetWorld();
	auto view_matrix = props.view;
	list->SetPipeline(data->pipeline_skylight);
	list->SetRenderTarget(data->output_buffer_resource);
	list->SetConstantBuffer("conf", data->constant_scene_buf_skylight);

	SkylightComponent* bg_comp = nullptr;

	for (auto& ent : skylight_view) {
		Entity entity = Entity((uint32_t)ent);
		auto& light = world.GetComponent<SkylightComponent>(entity);

		if (!light.GetReflectionMap() || light.GetReflectionMap()->GetStatus() != ReflectionMapStatus::LOADED) {
			continue;
		}

		if (light.IsBackgroundVisible()) {
			bg_comp = &light;
		}
	}

	if (bg_comp) {
		view_matrix[3] = glm::vec4(0.0f);
		view_matrix[3][3] = 1.0f;
		auto color_in = bg_comp->GetLightColor();
		glm::mat4 inverse_view_projection = glm::inverse(props.projection * view_matrix);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_bg, &inverse_view_projection, sizeof(glm::mat4), 0);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf_bg, glm::value_ptr(color_in), sizeof(glm::vec4), sizeof(glm::mat4));
		list->SetPipeline(data->pipeline_bg);
		list->SetTexture2DCubemap("in_tex", bg_comp->GetReflectionMap()->GetSpecularMap());
		list->SetVertexBuffer(data->card_mesh->GetVertexBuffer());
		list->SetConstantBuffer("mvp", data->constant_scene_buf_bg);
		list->SetIndexBuffer(data->card_mesh->GetIndexBuffer());
		list->Draw(data->card_mesh->GetIndexCount());
	}

}
