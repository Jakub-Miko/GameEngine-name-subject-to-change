#include "DeferredGeometryPass.h"
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

struct DeferredGeometryPass::internal_data {
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<Pipeline> pipeline_skeletal;
	std::shared_ptr<RenderBufferResource> constant_scene_buf;

	bool initialized = false;
};

struct PushConstantGeometryPassData {
	glm::mat4 mv;
#ifdef EDITOR
	uint32_t entity_id;
#endif
};

void DeferredGeometryPass::InitPostProcessingPassData() {
	GraphicsPipelineDescriptor pipeline_desc;
	pipeline_desc.viewport = RenderViewport();
	pipeline_desc.scissor_rect = RenderScissorRect();
	pipeline_desc.blend_functions = PipelineBlendFunctions();
	pipeline_desc.flags = PipelineFlags::ENABLE_DEPTH_TEST;
	pipeline_desc.depth_function = DepthFunction::LESS_EQUAL;
	pipeline_desc.layout = VertexLayoutFactory<MeshPreset>::GetLayout();
	pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
	pipeline_desc.framebuffer_format.color_attachemt_formats = {
        { TextureFormat::RGBA_UNSIGNED_CHAR },
		{ TextureFormat::RG_16FLOAT},
		{ TextureFormat::RGBA_16FLOAT}
    };
#ifdef EDITOR
	
	pipeline_desc.framebuffer_format.color_attachemt_formats.push_back({TextureFormat::R_UNSIGNED_INT});
	//Use a shader with ids, for viewport picking in the editor.
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/GeometryPassShaderEditor.glsl");
#else 
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/GeometryPassShader.glsl");
#endif

	data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

	pipeline_desc.layout = VertexLayoutFactory<SkeletalGeometryPassPreset>::GetLayout();

#ifdef EDITOR
	//Use a shader with ids, for viewport picking in the editor.
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/SkeletalGeometryPassShaderEditor.glsl");
#else
	pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/SkeletalGeometryPassShader.glsl");
#endif

	data->pipeline_skeletal = PipelineManager::Get()->CreatePipeline(pipeline_desc);

#ifdef EDITOR
	//we need to pass an extra entity id 
	RenderBufferDescriptor const_desc(sizeof(glm::mat4), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
#else 
	RenderBufferDescriptor const_desc(sizeof(glm::mat4) * 2, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
#endif
	data->constant_scene_buf = RenderResourceManager::Get()->CreateBuffer(const_desc);


	data->initialized = true;
}

void DeferredGeometryPass::RenderGeometry(std::shared_ptr<RenderCommandList> list, render_props& props) {
	auto& geometry = props.resource_manager.GetResource<RenderResourceCollection<Entity>>(input_geometry);
	auto& world = Application::GetWorld();

	list->SetPipeline(data->pipeline);
	list->SetRenderTarget(props.output_buffer);
	list->SetConstantBuffer("mvp", data->constant_scene_buf);

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
		PushConstantGeometryPassData push_data = {};
		push_data.mv = mv_matrix;
#ifdef EDITOR
		push_data.entity_id = entity.id;
#endif
		list->SetPushConstantRange(&push_data, sizeof(PushConstantGeometryPassData));
		list->Draw(mesh.GetMesh()->GetIndexCount());
	}
}

void DeferredGeometryPass::RenderSkeletalGeometry(std::shared_ptr<RenderCommandList> list, render_props& props) {
	auto& geometry = props.resource_manager.GetResource<RenderResourceCollection<Entity>>(input_skeletal_geometry);
	auto& world = Application::GetWorld();
	list->SetPipeline(data->pipeline_skeletal);
	list->SetRenderTarget(props.output_buffer);
	list->SetConstantBuffer("mvp", data->constant_scene_buf);

	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
		world.UpdateSkeletalMesh(entity);
		mesh.GetAnimation().UpdateAnimation(Application::GetDeltaTime(), list, mesh.GetMesh());
	}

	for(auto& entity : geometry.resources) {
		auto& mesh = world.GetComponent<SkeletalMeshComponent>(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);

		if (mesh.material == nullptr) {
			props.default_material->SetMaterial(list);
		}
		else {
			mesh.material->SetMaterial(list);
		}
		list->SetVertexBuffer(mesh.GetMesh()->GetVertexBuffer());
		list->SetIndexBuffer(mesh.GetMesh()->GetIndexBuffer());

		list->SetConstantBuffer("bones", mesh.GetAnimation().GetBoneBuffer());

		glm::mat4 mv_matrix = props.view_matrix * transform.TransformMatrix;

		PushConstantGeometryPassData push_data = {};
		push_data.mv = mv_matrix;
#ifdef EDITOR
		push_data.entity_id = entity.id;
#endif
		list->SetPushConstantRange(&push_data, sizeof(PushConstantGeometryPassData));

		list->Draw(mesh.GetMesh()->GetIndexCount());
	}
}


DeferredGeometryPass::DeferredGeometryPass(const std::string& input_geometry,const std::string& input_skeletal_geometry, const std::string& input_buffer,
	const std::string& output_buffer) : input_geometry(input_geometry), output_buffer(output_buffer), input_buffer(input_buffer),
	input_skeletal_geometry(input_skeletal_geometry)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void DeferredGeometryPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<RenderResourceCollection<Entity>>(input_skeletal_geometry, RenderPassResourceDescriptor_Access::READ);
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
}

void DeferredGeometryPass::Render(RenderPipelineResourceManager& resource_manager)
{
	PROFILE("GeometryPass");
	auto list = Renderer::Get()->GetRenderCommandList();
	auto& world = Application::GetWorld();
	auto& camera = world.GetComponent<CameraComponent>(world.GetPrimaryEntity());
	auto& camera_trans = world.GetComponent<TransformComponent>(world.GetPrimaryEntity());
	camera.UpdateProjectionMatrix();

	render_props props(resource_manager);
	props.output_buffer = resource_manager.GetResource<std::shared_ptr<RenderFrameBufferResource>>(input_buffer);
	props.projection_matrix = camera.GetProjectionMatrix();
	props.view_matrix = glm::inverse(camera_trans.TransformMatrix);
	props.default_material = MaterialManager::Get()->GetMaterialTemplate("DeferredGPassMaterial")->GetDefaultMaterial();

	RenderResourceManager::Get()->UploadDataToBuffer(list, data->constant_scene_buf, glm::value_ptr(props.projection_matrix), sizeof(glm::mat4), 0);

	RenderGeometry(list, props);
	RenderSkeletalGeometry(list, props);

	Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, props.output_buffer);
}

DeferredGeometryPass::~DeferredGeometryPass()
{
	if (data) {
		delete data;
	}
}