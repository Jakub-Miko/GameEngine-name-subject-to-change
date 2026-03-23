#include "ClusteredForwardPass.h"
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
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <Application.h>
#include <Window.h>

#include "ClusteredLightCullingPass.h"

struct GeometryPassPreset;

template<>
struct VertexLayoutFactory<GeometryPassPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "normal"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4, "tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv")
				});


			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

struct render_props {
	std::shared_ptr<RenderFrameBufferResource> output_buffer;
	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;
	std::shared_ptr<Material> default_material;
};

struct ClusteredForwardPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<Pipeline> pipeline_skeletal;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;

	bool initialized = false;
};

struct ConfigData {
	glm::mat4 projection_matrix;
	glm::mat4 inverse_projection;;
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

struct PushConstantForwardPassData {
	glm::mat4 mv;
#ifdef EDITOR
	uint32_t id;
#endif
};

void ClusteredForwardPass::InitPostProcessingPassData() {
	GraphicsPipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.depth_function = DepthFunction::LESS_EQUAL;
	pipeline_desc.layout = VertexLayoutFactory<MeshPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.framebuffer_format.color_attachemt_formats = {
		{ TextureFormat::RGBA_16FLOAT}
    };

#ifdef EDITOR

	pipeline_desc.framebuffer_format.color_attachemt_formats.push_back({TextureFormat::R_UNSIGNED_INT});
	//Use a shader with ids, for viewport picking in the editor.
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredForwardPassShaderEditor.glsl",  {{"HARD_CODE_CASCADES", HARD_CODE_CASCADES}});
#else
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredForwardPassShader.glsl",  {{"HARD_CODE_CASCADES", HARD_CODE_CASCADES}});
#endif


	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.layout = VertexLayoutFactory<SkeletalGeometryPassPreset>::GetLayout();


	// pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ClusteredRenderer/ClusteredForwardSkeletalPassShader.glsl");
	//
	// data->pipeline_skeletal = PipelineManager::Get()->CreatePipeline(pipeline_desc);


	RenderBufferDescriptor const_desc(sizeof(ConfigData), RenderBufferType::DEFAULT, RenderBufferUsage::CONSTANT_BUFFER);

	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);

	data->initialized = true;
}

void ClusteredForwardPass::RenderGeometry(std::shared_ptr<RenderCommandList> list, render_props& props) {
	auto& geometry = props.resource_manager.GetResource<RenderResourceCollection<Entity>>(input_geometry);
	auto& world = Application::GetWorld();
	auto& clustered_lights = props.resource_manager.GetResource<ClusteredLightLists>(input_clustered_lights);
	auto& point_shadow_maps = props.resource_manager.GetPersistentResource<std::shared_ptr<RenderResourceStore>>(input_point_shadow_maps);
	auto& directional_shadow_maps = props.resource_manager.GetPersistentResource<std::shared_ptr<RenderResourceStore>>(input_directional_shadow_maps);

	list->SetPipeline(data->pipeline);
	list->SetRenderTarget(props.output_buffer);

	list->SetConstantBuffer("conf", data->constant_scene_buf);
	list->SetStorageBuffer("point_light_buffer", clustered_lights.point_light_buffer);
	list->SetStorageBuffer("directional_light_buffer", clustered_lights.directional_light_buffer);
	list->SetStorageBuffer("skylight_buffer", clustered_lights.skylight_buffer);
	list->SetStorageBuffer("light_assignment_buffer", clustered_lights.light_assignment_buffer);
	list->SetStorageBuffer("cluster_buffer", clustered_lights.cluster_buffer);
	list->SetResourceStore("point_light_shadow_maps", point_shadow_maps);
	list->SetResourceStore("directional_light_shadow_maps", directional_shadow_maps);
	list->SetResourceStore("skylight_reflection_maps", TextureManager::Get()->GetReflectionMapResourceStore());

	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<MeshComponent>(entity);
		world.UpdateMesh(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);

		if (mesh.material == nullptr) {
			props.default_material->SetMaterial(list);
		}
		else {
			mesh.material->SetMaterial(list);
		}
		list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
		list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());
		glm::mat4 mv_matrix = props.view_matrix * transform.TransformMatrix;
		PushConstantForwardPassData push_data = {};
		push_data.mv = mv_matrix;
#ifdef EDITOR
		push_data.id = entity.id;
#endif
		list->SetPushConstantRange(&push_data, sizeof(PushConstantForwardPassData));
		list->Draw(mesh.GetMesh()->GetIndexCount());
	}
}

void ClusteredForwardPass::RenderSkeletalGeometry(std::shared_ptr<RenderCommandList> list, render_props& props) {
	// auto& geometry = props.resource_manager.GetResource<RenderResourceCollection<Entity>>(input_skeletal_geometry);
	// auto& world = Application::GetWorld();
	// list->SetPipeline(data->pipeline_skeletal);
	// list->SetRenderTarget(props.output_buffer);
	// list->SetConstantBuffer("mvp", data->constant_scene_buf);
	//
	// for(auto& entity : geometry.resources) {
	// 	auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
	// 	world.UpdateSkeletalMesh(entity);
	// 	mesh.GetAnimation().UpdateAnimation(Application::GetDeltaTime(), list, mesh.GetMesh());
	// }
	//
	// for(auto& entity : geometry.resources) {
	// 	auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
	// 	auto& transform = world.GetComponent<TransformComponent>(entity);
	//
	// 	if (mesh.material == nullptr) {
	// 		props.default_material->SetMaterial(list);
	// 	}
	// 	else {
	// 		mesh.material->SetMaterial(list);
	// 	}
	// 	list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
	// 	list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());
	//
	// 	list->SetConstantBuffer("bones", mesh.GetAnimation().GetBoneBuffer());
	//
	// 	glm::mat4 mv_matrix = props.view_matrix * transform.TransformMatrix;
	//
	// 	PushConstantForwardPassData push_data = {};
	// 	push_data.mv = mv_matrix;
	// 	list->SetPushConstantRange(&push_data, sizeof(PushConstantForwardPassData));
	//
	// 	list->Draw(mesh.GetMesh()->GetIndexCount());
	// }
}


ClusteredForwardPass::ClusteredForwardPass(const std::string& input_geometry, const std::string& input_color_buffer,
	const std::string& input_skeletal_geometry, const std::string& input_clustered_lights,
	const std::string& input_directional_shadowed_lights, const std::string& input_point_shadowed_lights,
	const std::string& output_buffer, const std::string& shadow_map_dependency_tag,
	const std::string& input_point_shadow_maps, const std::string& input_directional_shadow_maps)
		: input_geometry(input_geometry), input_skeletal_geometry(input_skeletal_geometry), input_clustered_lights(input_clustered_lights),
		input_directional_shadowed_lights(input_directional_shadowed_lights), input_point_shadowed_lights(input_point_shadowed_lights),
		output_buffer(output_buffer), shadow_map_dependency_tag(shadow_map_dependency_tag),
		input_point_shadow_maps(input_point_shadow_maps), input_directional_shadow_maps(input_directional_shadow_maps), input_color_buffer(input_color_buffer) {
	data = new internal_data;
	InitPostProcessingPassData();
}

void ClusteredForwardPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_skeletal_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<ClusteredLightLists>(input_clustered_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_directional_shadowed_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_point_shadowed_lights, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<DependencyTag>(shadow_map_dependency_tag, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_color_buffer, RenderPassResourceDescriptor_Access::READ);

}

void ClusteredForwardPass::Render(RenderPipelineResourceManager& resource_manager)
{
	PROFILE("ForwardPass");

	auto& clustered_lights = resource_manager.GetResource<ClusteredLightLists>(input_clustered_lights);

	if(clustered_lights.num_of_point_lights == 0 && clustered_lights.num_of_directional_lights == 0) return;

	auto list = Renderer::Get()->GetRenderCommandList();
	auto& world = Application::GetWorld();
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	camera.UpdateProjectionMatrix();

	render_props props(resource_manager);
	props.default_material = MaterialManager::Get()->GetMaterialTemplate("DeferredGPassMaterial")->GetDefaultMaterial();
	props.view_matrix = glm::inverse(camera_trans.TransformMatrix);
	props.output_buffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_color_buffer);

	glm::vec2 pixel_size = { 1.0f / Application::Get()->GetWindow()->GetProperties().resolution_x,
		1.0f / Application::Get()->GetWindow()->GetProperties().resolution_y };


	ConfigData config_data = {};
	config_data.point_light_count = clustered_lights.num_of_point_lights;
	config_data.directional_light_count = clustered_lights.num_of_directional_lights;
	config_data.skylight_count = clustered_lights.num_of_skylights;
	config_data.cluster_grid_size = glm::uvec3(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	config_data.depth_constant_a = camera.zFar / (camera.zFar - camera.zNear);;
	config_data.depth_constant_b = (-camera.zFar * camera.zNear) / (camera.zFar - camera.zNear);;
	config_data.far_plane = camera.zFar;
	config_data.near_plane = camera.zNear;
	config_data.pixel_size = pixel_size;
	config_data.projection_matrix = camera.GetProjectionMatrix();
	config_data.inverse_projection = glm::inverse(config_data.projection_matrix);
	config_data.inverse_view_matrix = camera_trans.TransformMatrix;
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, &config_data, sizeof(ConfigData), 0);

	RenderGeometry(list, props);
	RenderSkeletalGeometry(list, props);

	Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);

	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, props.output_buffer);
}

ClusteredForwardPass::~ClusteredForwardPass()
{
	if (data) {
		delete data;
	}
}