#include "DefferedSkeletalGeometryPass.h"
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
#include <World/Components/SkeletalMeshComponent.h>
#include <Application.h>
#include <Window.h>

#ifndef MAX_NUM_OF_BONES
#define MAX_NUM_OF_BONES 80
#endif

static const int max_num_of_bones = MAX_NUM_OF_BONES;

struct SkeletalGeometryPassPreset;

template<>
struct VertexLayoutFactory<SkeletalGeometryPassPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::UNSIGNED_INT,4,"bone_ids"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4,"bone_weights"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "normal"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv0")
				});


			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};


struct DefferedSkeletalGeometryPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;
	std::shared_ptr<RenderBufferResource> constant_bone_buf;
	bool initialized = false;
};

void DefferedSkeletalGeometryPass::InitPostProcessingPassData() {
	PipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.layout = VertexLayoutFactory<SkeletalGeometryPassPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
#ifdef EDITOR

	//Use a shader with ids, for viewport picking in the editor.
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/SkeletalGeometryPassShaderEditor.glsl");
#else 
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/SkeletalGeometryPassShader.glsl");
#endif

	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

#ifdef EDITOR
	//we need to pass an extra entity id 
	RenderBufferDescriptor const_desc(sizeof(glm::mat4) * 2 + sizeof(uint32_t), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
#else 
	RenderBufferDescriptor const_desc(sizeof(glm::mat4) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
#endif
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);

	RenderBufferDescriptor const_bone_desc(sizeof(glm::mat4) * max_num_of_bones + sizeof(unsigned int), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
	data->constant_bone_buf = RenderResourceManager::Get()->CreateBuffer(const_bone_desc);


	data->initialized = true;
}


DefferedSkeletalGeometryPass::DefferedSkeletalGeometryPass(const std::string& input_geometry, const std::string& input_buffer, const std::string& output_buffer) : input_geometry(input_geometry), output_buffer(output_buffer), input_buffer(input_buffer)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void DefferedSkeletalGeometryPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
}

void DefferedSkeletalGeometryPass::Render(RenderPipelineResourceManager& resource_manager)
{
	auto& geometry = resource_manager.GetResource<RenderResourceCollection<Entity>>(input_geometry);
	auto& out_buffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer);
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
	list->SetRenderTarget(out_buffer);
	list->SetConstantBuffer("mvp", data->constant_scene_buf);
	list->SetConstantBuffer("bones", data->constant_bone_buf);
	auto default_mat = data->pipeline->GetShader()->GetDefaultMaterial();
	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);

		if (mesh.material == nullptr) {
			default_mat->SetMaterial(list, data->pipeline);
		}
		else {
			mesh.material->SetMaterial(list, data->pipeline);
		}
		list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
		list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());

		mesh.GetAnimation().UpdateAnimation(Application::GetDeltaTime(), data->constant_bone_buf, list, mesh.GetMesh());

		glm::mat4 mvp = ViewProjection * transform.TransformMatrix;
		glm::mat4 mv_matrix = view_matrix * transform.TransformMatrix;
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mvp), sizeof(glm::mat4), 0);
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(mv_matrix), sizeof(glm::mat4), sizeof(glm::mat4));

#ifdef EDITOR
		//Pass the extra entity id
		RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, (void*)&entity.id, sizeof(uint32_t), sizeof(glm::mat4) * 2);
#endif

		list->Draw(mesh.GetMesh()->GetIndexCount());


	}

	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, out_buffer);


}

DefferedSkeletalGeometryPass::~DefferedSkeletalGeometryPass()
{
	if (data) {
		delete data;
	}
}