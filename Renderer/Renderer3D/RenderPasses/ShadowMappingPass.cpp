#include "ShadowMappingPass.h"
#include <Application.h>
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

struct ShadowMappingPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<TextureSampler> depth_sampler;
	std::shared_ptr<RenderBufferResource> const_buffer;
};

ShadowMappingPass::ShadowMappingPass(const std::string& input_shadow_casters, const std::string& output_dependency_tag) : input_shadow_casters(input_shadow_casters), output_dependency_tag(output_dependency_tag)
{
	data = new internal_data;
	InitShadowMappingPassData();
}

void ShadowMappingPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_shadow_casters, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<DependencyTag>(output_dependency_tag, RenderPassResourceDescriptor_Access::WRITE);
}

void ShadowMappingPass::Render(RenderPipelineResourceManager& resource_manager)
{
	auto& shadow_caster_entities = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_shadow_casters);
	auto& world = Application::GetWorld();
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	list->SetPipeline(data->pipeline);
	for (auto& caster : shadow_caster_entities.resources) {
		auto& shadow_comp = world.GetComponent<ShadowCasterComponent>(caster);
		auto& caster_trans_comp = world.GetComponent<TransformComponent>(caster);
		auto& caster_light_comp = world.GetComponent<LightComponent>(caster);

		if (shadow_comp.shadow_map == nullptr) {
			InitShadowComponent(caster);
		}

		list->SetRenderTarget(shadow_comp.shadow_map);


		glm::vec3 size = caster_trans_comp.size;
		glm::mat3 rotation_matrix;
		rotation_matrix[0] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[0]);
		rotation_matrix[1] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[1]);
		rotation_matrix[2] = glm::normalize((glm::vec3)caster_trans_comp.TransformMatrix[2]);
		glm::vec3 translation = (rotation_matrix * glm::vec3(0.0f, 0.0f, -glm::abs(size.z)/2.0f)) + (glm::vec3)caster_trans_comp.TransformMatrix[3];
		glm::mat4 projection = glm::ortho(-size.r / 2, size.r / 2, -size.g / 2, size.g / 2, 0.0f, size.b);

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
}

ShadowMappingPass::~ShadowMappingPass()
{
	if (data) {
		delete data;
	}
}

void ShadowMappingPass::InitShadowComponent(Entity ent)
{
	auto& shadow_comp = Application::GetWorld().GetComponent<ShadowCasterComponent>(ent);
	RenderTexture2DDescriptor depth_text_desc;
	depth_text_desc.height = shadow_comp.res_y;
	depth_text_desc.width = shadow_comp.res_x;
	depth_text_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
	depth_text_desc.sampler = data->depth_sampler;
	
	auto depth_text = RenderResourceManager::Get()->CreateTexture(depth_text_desc);

	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = {};
	framebuffer_desc.depth_stencil_attachment = depth_text;

	shadow_comp.shadow_map = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);

	RenderBufferDescriptor desc(sizeof(glm::mat4), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->const_buffer = RenderResourceManager::Get()->CreateBuffer(desc);

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
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/ShadowMappingShader.glsl");

	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	TextureSamplerDescritor sample_desc;
	sample_desc.AddressMode_U = TextureAddressMode::CLAMP;
	sample_desc.AddressMode_V = TextureAddressMode::CLAMP;
	sample_desc.AddressMode_W = TextureAddressMode::CLAMP;
	sample_desc.border_color = glm::vec4(1.0);
	sample_desc.filter = TextureFilter::LINEAR_MIN_MAG;

	data->depth_sampler = TextureSampler::CreateSampler(sample_desc);
		

}
