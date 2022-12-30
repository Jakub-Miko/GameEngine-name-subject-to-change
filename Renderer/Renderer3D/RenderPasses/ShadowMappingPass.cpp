#include "ShadowMappingPass.h"
#include <Application.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/LightComponent.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <World/World.h>
#include <Renderer/Renderer3D/RenderResourceCollection.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/PipelineManager.h>

#define HARD_CODE_CASCADES 5

struct ShadowMappingPass::internal_data {
	std::shared_ptr<Pipeline> pipeline_directional;
	std::shared_ptr<Pipeline> pipeline_point;
	std::shared_ptr<TextureSampler> depth_sampler_directional;
	std::shared_ptr<TextureSampler> depth_sampler_point;
	std::shared_ptr<RenderBufferResource> const_buffer_directional;
	std::shared_ptr<RenderBufferResource> const_buffer_point;
	RenderResourceCollection<glm::mat4> shadow_cascades;
};

ShadowMappingPass::ShadowMappingPass(const std::string& input_shadow_casters_directional, const std::string& input_shadow_casters_point, const std::string& output_dependency_tag, const std::string& output_shadow_cascades)
	: input_shadow_casters_directional(input_shadow_casters_directional), input_shadow_casters_point(input_shadow_casters_point), output_dependency_tag(output_dependency_tag), output_shadow_cascades(output_shadow_cascades)
{
	data = new internal_data;
	InitShadowMappingPassData();
}

void ShadowMappingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_shadow_casters_directional, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_shadow_casters_point, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<DependencyTag>(output_dependency_tag, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddResource<RenderResourceCollection<glm::mat4>>(output_shadow_cascades, RenderPassResourceDescriptor_Access::WRITE);
}

void ShadowMappingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	/*auto& shadow_caster_entities = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadow_casters_directional);
	auto& world = Application::GetWorld();
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	list->SetPipeline(data->pipeline);
	for (auto& caster : shadow_caster_entities.resources) {
		auto& shadow_comp = world.GetComponent<ShadowCasterComponent>(caster);
		auto& caster_trans_comp = world.GetComponent<TransformComponent>(caster);
		auto& caster_light_comp = world.GetComponent<LightComponent>(caster);

		if (shadow_comp.shadow_map == nullptr) {
			InitShadowComponent(caster, caster_light_comp.type);
		}

		list->SetRenderTarget(shadow_comp.shadow_map);


		glm::vec3 size = caster_trans_comp.size;
		glm::mat3 rotation_matrix;
		rotation_matrix[0] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[0]);
		rotation_matrix[1] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[1]);
		rotation_matrix[2] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[2]);
		size.b = shadow_comp.far_plane - shadow_comp.near_plane;
		glm::vec3 translation = (rotation_matrix * glm::vec3(0.0f, 0.0f, -shadow_comp.near_plane - glm::abs(size.b)/2.0f)) + (glm::vec3)caster_trans_comp.TransformMatrix[3];
		glm::mat4 projection = glm::ortho(-size.r / 2, size.r / 2, -size.g / 2, size.g / 2, shadow_comp.near_plane, shadow_comp.far_plane);

		list->SetConstantBuffer("mvp", data->const_buffer);

		OrientedBoundingBox culling_box(size, translation, rotation_matrix);

		std::vector<Entity> entities;
		Application::GetWorld().GetSpatialIndex().BoxCulling(Application::GetWorld(), culling_box, entities);

		list->Clear();
		glm::mat4 light_view = glm::translate(glm::mat4(1.0), (glm::vec3)caster_trans_comp.TransformMatrix[3]) * (glm::mat4)rotation_matrix;
		glm::mat4 view_projection = projection * glm::inverse(light_view);

		shadow_comp.light_view_matrix = view_projection;

		list->SetViewport(RenderViewport(glm::vec2(0.0f), glm::vec2(shadow_comp.res_x, shadow_comp.res_y)));

		for (auto& entity : entities) {
			if (world.HasComponent<MeshComponent>(entity)) {
				auto& mesh = world.GetComponent<MeshComponent>(entity);
				auto& trans = world.GetComponent<TransformComponent>(entity);
				glm::mat4 mvp = view_projection * trans.TransformMatrix;
				RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer, glm::value_ptr(mvp), sizeof(glm::mat4), 0);
				
				list->SetVertexBuffer(mesh.mesh->GetVertexBuffer());
				list->SetIndexBuffer(mesh.mesh->GetIndexBuffer());
				list->Draw(mesh.mesh->GetIndexCount());

			}
		}


	}
	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<DependencyTag>(output_dependency_tag, DependencyTag());*/

	Render_impl<LightType::DIRECTIONAL>(resource_manager);
	Render_impl<LightType::POINT>(resource_manager);

}

ShadowMappingPass::~ShadowMappingPass()
{
	if (data) {
		delete data;
	}
}

void ShadowMappingPass::InitShadowComponent(Entity ent, LightType light_type)
{
	auto& shadow_comp = Application::GetWorld().GetComponent<ShadowCasterComponent>(ent);
	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = {};
	if (light_type == LightType::DIRECTIONAL) {
		RenderTexture2DArrayDescriptor desc;
		desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
		desc.height = shadow_comp.res_y;
		desc.width = shadow_comp.res_x;
		desc.num_of_textures = HARD_CODE_CASCADES;
		desc.sampler = data->depth_sampler_directional;
		
		auto depth_array = RenderResourceManager::Get()->CreateTextureArray(desc);
		
		//RenderTexture2DDescriptor depth_text_desc;
		//depth_text_desc.height = shadow_comp.res_y;
		//depth_text_desc.width = shadow_comp.res_x;
		//depth_text_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
		//depth_text_desc.sampler = data->depth_sampler_directional;
		//
		//auto depth_text = RenderResourceManager::Get()->CreateTexture(depth_text_desc);

		framebuffer_desc.depth_stencil_attachment = depth_array;
	}
	else if(light_type == LightType::POINT){
		RenderTexture2DCubemapDescriptor depth_text_desc;
		shadow_comp.res_y = shadow_comp.res_x;
		depth_text_desc.res = shadow_comp.res_x;
		depth_text_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
		depth_text_desc.sampler = data->depth_sampler_point;

		auto depth_text = RenderResourceManager::Get()->CreateTextureCubemap(depth_text_desc);
		framebuffer_desc.depth_stencil_attachment = depth_text;
	}


	shadow_comp.shadow_map = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);
	shadow_comp.cascades = HARD_CODE_CASCADES;
}

void ShadowMappingPass::RenderDirectionalShadowCaster(Entity caster, RenderCommandList* list)
{

	auto& world = Application::GetWorld();
	auto& shadow_comp = world.GetComponent<ShadowCasterComponent>(caster);
	auto& caster_trans_comp = world.GetComponent<TransformComponent>(caster);
	auto& caster_light_comp = world.GetComponent<LightComponent>(caster);
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	auto& camera_transform = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	if (shadow_comp.shadow_map == nullptr) {
		InitShadowComponent(caster, LightType::DIRECTIONAL);
	}

	list->SetRenderTarget(shadow_comp.shadow_map);
	glm::mat3 rotation_matrix;
	rotation_matrix[0] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[0]);
	rotation_matrix[1] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[1]);
	rotation_matrix[2] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[2]);
	glm::mat4 shadow_matricies[HARD_CODE_CASCADES];

	float cascade_levels[HARD_CODE_CASCADES + 1];
	for (int i = 0; i < HARD_CODE_CASCADES + 1; i++) {
		cascade_levels[i] = camera.zNear + (((camera.zFar - camera.zNear) / (HARD_CODE_CASCADES)) * i);
	}
	for (int cascade = 0; cascade < HARD_CODE_CASCADES; cascade++) {

		float near = cascade_levels[cascade];
		float far = cascade_levels[cascade + 1];

		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, near, far);;
		glm::mat4 camera_inverse_view_projection = glm::inverse(projection * glm::inverse(camera_transform.TransformMatrix));

		glm::vec3 frustum_points[8];
		glm::vec3 center = glm::vec3(0.0f);
		int i = 0;
		for (int x = 0; x < 2; x++) {
			for (int y = 0; y < 2; y++) {
				for (int z = 0; z < 2; z++) {
					glm::vec4 point = camera_inverse_view_projection * glm::vec4(x * 2.0 - 1.0, y * 2.0 - 1.0, z * 2.0 - 1.0, 1.0f);
					frustum_points[i] = glm::vec3(point / point.w);
					center += frustum_points[i];
					i++;
				}
			}
		}
		center /= 8;
		glm::mat4 shadow_matrix_view = glm::lookAt(center + glm::vec3(0.0f, 0.0f, 1.0f) * glm::inverse(rotation_matrix), center, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
		for (auto& corner : frustum_points) {
			glm::vec4 transformed_corner = shadow_matrix_view * glm::vec4(corner, 1.0f);
			for (int i = 0; i < 3; i++) {
				min[i] = std::min(min[i], transformed_corner[i]);
				max[i] = std::max(max[i], transformed_corner[i]);
			}
		}
		glm::mat4 shadow_projection = glm::ortho(min.x, max.x, min.y, max.y, -max.z - shadow_comp.far_plane, -min.z);
		shadow_matricies[cascade] = shadow_projection * shadow_matrix_view;
		data->shadow_cascades.resources.push_back(shadow_matricies[cascade]);
	}

	std::vector<Entity> entities;
	glm::mat4 view_projection;

	glm::vec3 size = caster_trans_comp.size;
	size.b = shadow_comp.far_plane - shadow_comp.near_plane;
	glm::vec3 translation = (rotation_matrix * glm::vec3(0.0f, 0.0f, -shadow_comp.near_plane - glm::abs(size.b) / 2.0f)) + (glm::vec3)caster_trans_comp.TransformMatrix[3];
	glm::mat4 projection = glm::ortho(-size.r / 2, size.r / 2, -size.g / 2, size.g / 2, shadow_comp.near_plane, shadow_comp.far_plane);

	list->SetConstantBuffer("mvp", data->const_buffer_directional);
	OrientedBoundingBox culling_box(glm::vec3(500000), glm::vec3(0.0f));

	Application::GetWorld().GetSpatialIndex().BoxCulling(Application::GetWorld(), culling_box, entities);

	glm::mat4 light_view = glm::translate(glm::mat4(1.0), (glm::vec3)caster_trans_comp.TransformMatrix[3]) * (glm::mat4)rotation_matrix;
	view_projection = projection * glm::inverse(light_view);

	shadow_comp.light_view_matrix = view_projection;
	list->Clear();
	list->SetViewport(RenderViewport(glm::vec2(0.0f), glm::vec2(shadow_comp.res_x, shadow_comp.res_y)));

	for (auto& entity : entities) {
		if (world.HasComponent<MeshComponent>(entity)) {
			auto& mesh = world.GetComponent<MeshComponent>(entity);
			auto& trans = world.GetComponent<TransformComponent>(entity);
			glm::mat4 mvp = view_projection * trans.TransformMatrix;
			RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_directional, glm::value_ptr(trans.TransformMatrix), sizeof(glm::mat4), 0);
			for (int i = 0; i < HARD_CODE_CASCADES; i++) {
				RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_directional, glm::value_ptr(shadow_matricies[i]), sizeof(glm::mat4), sizeof(glm::mat4) * (1+i));
			}
			list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
			list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());
			list->Draw(mesh.GetMesh()->GetIndexCount());

		}
	}

}

void ShadowMappingPass::RenderPointShadowCaster(Entity caster, RenderCommandList* list)
{
	auto& world = Application::GetWorld();
	auto& shadow_comp = world.GetComponent<ShadowCasterComponent>(caster);
	auto& caster_trans_comp = world.GetComponent<TransformComponent>(caster);
	auto& caster_light_comp = world.GetComponent<LightComponent>(caster);

	if (shadow_comp.shadow_map == nullptr) {
		InitShadowComponent(caster, LightType::POINT);
	}

	list->SetRenderTarget(shadow_comp.shadow_map);
	std::vector<Entity> entities;
	glm::mat4 view_projection;

	glm::vec3 translation = (glm::vec3)caster_trans_comp.TransformMatrix[3];
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, shadow_comp.near_plane, shadow_comp.far_plane);

	BoundingSphere culling_sphere(shadow_comp.far_plane, translation);

	Application::GetWorld().GetSpatialIndex().SphereCulling(Application::GetWorld(), culling_sphere, entities);
	list->SetConstantBuffer("mvp", data->const_buffer_point);

	glm::mat4 light_views[6];
	light_views[0] = projection * glm::lookAt(translation, translation + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	light_views[1] = projection * glm::lookAt(translation, translation + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	light_views[2] = projection * glm::lookAt(translation, translation + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	light_views[3] = projection * glm::lookAt(translation, translation + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	light_views[4] = projection * glm::lookAt(translation, translation + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	light_views[5] = projection * glm::lookAt(translation, translation + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_point, light_views, sizeof(glm::mat4) * 6, sizeof(glm::mat4));
	glm::vec4 trans = glm::vec4(translation, 1.0f);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_point, glm::value_ptr(trans), sizeof(glm::vec4), sizeof(glm::mat4) * 7);
	RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_point, &shadow_comp.far_plane, sizeof(float), sizeof(glm::mat4) * 7 + sizeof(glm::vec4));
	view_projection = glm::inverse(glm::translate(glm::mat4(1.0f), translation));



	shadow_comp.light_view_matrix = view_projection;
	list->Clear();
	list->SetViewport(RenderViewport(glm::vec2(0.0f), glm::vec2(shadow_comp.res_x, shadow_comp.res_y)));

	for (auto& entity : entities) {
		if (world.HasComponent<MeshComponent>(entity)) {
			auto& mesh = world.GetComponent<MeshComponent>(entity);
			auto& trans = world.GetComponent<TransformComponent>(entity);
			glm::mat4 model = trans.TransformMatrix;
			RenderResourceManager::Get()->UploadDataToBuffer(list, data->const_buffer_point, glm::value_ptr(model), sizeof(glm::mat4), 0);
			list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
			list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());
			list->Draw(mesh.GetMesh()->GetIndexCount());

		}
	}


}

void ShadowMappingPass::InitShadowMappingPassData()
{
	PipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.layout = VertexLayoutFactory<MeshPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ShadowMappingShaderDirectional.glsl");

	data->pipeline_directional = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ShadowMappingShaderPoint.glsl");

	data->pipeline_point = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	TextureSamplerDescritor sample_desc;
	sample_desc.AddressMode_U = TextureAddressMode::BORDER;
	sample_desc.AddressMode_V = TextureAddressMode::BORDER;
	sample_desc.AddressMode_W = TextureAddressMode::BORDER;
	sample_desc.border_color = glm::vec4(1.0);
	sample_desc.filter = TextureFilter::LINEAR_MIN_MAG;

	data->depth_sampler_directional = TextureSampler::CreateSampler(sample_desc);
	sample_desc.AddressMode_U = TextureAddressMode::CLAMP;
	sample_desc.AddressMode_V = TextureAddressMode::CLAMP;
	sample_desc.AddressMode_W = TextureAddressMode::CLAMP;
	data->depth_sampler_point = TextureSampler::CreateSampler(sample_desc);
		
	RenderBufferDescriptor desc(sizeof(glm::mat4) * (HARD_CODE_CASCADES +1), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->const_buffer_directional = RenderResourceManager::Get()->CreateBuffer(desc);


	data->shadow_cascades.resources.reserve(20);

	RenderBufferDescriptor desc_point(sizeof(glm::mat4)*7 + sizeof(glm::vec4) + sizeof(float), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->const_buffer_point = RenderResourceManager::Get()->CreateBuffer(desc_point);

}

template<LightType type>
void ShadowMappingPass::Render_impl(RenderPipelineResourceManager& resource_manager)
{
	const RenderResourceCollection<Entity>* shadow_caster_entities;
	if constexpr (type == LightType::DIRECTIONAL) {
		data->shadow_cascades.resources.clear();
		shadow_caster_entities = &resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadow_casters_directional);
	}
	else if constexpr (type == LightType::POINT) {
		shadow_caster_entities = &resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadow_casters_point);
	}
	
	auto& world = Application::GetWorld();
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();

	if constexpr (type == LightType::DIRECTIONAL) {
		list->SetPipeline(data->pipeline_directional);
	}
	else if constexpr (type == LightType::POINT) {
		list->SetPipeline(data->pipeline_point);
	}
	
	for (auto& caster : shadow_caster_entities->resources) {
		if constexpr (type == LightType::POINT) {
			RenderPointShadowCaster(caster, list);
		}
		else if constexpr (type == LightType::DIRECTIONAL) {
			RenderDirectionalShadowCaster(caster, list);
		}
	}
	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<DependencyTag>(output_dependency_tag, DependencyTag());
	if constexpr (type == LightType::DIRECTIONAL) {
		resource_manager.SetResource<RenderResourceCollection<glm::mat4>>(output_shadow_cascades,data->shadow_cascades);
	}
}
