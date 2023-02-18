#include "GenerateGBufferPass.h"
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/Renderer3D/RenderPipeline.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RenderResourceManager.h>
#include <Application.h>
#include <Window.h>

struct GenerateGBufferPass::internal_data {
	std::shared_ptr<RenderFrameBufferResource> output_buffer_resource;
#ifdef EDITOR
	int id_texture;
#endif


	bool initialized = false;
};

void GenerateGBufferPass::InitPostProcessingPassData() {

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

	RenderTexture2DDescriptor roughness_texture_desc;
	roughness_texture_desc.format = TextureFormat::R_8FLOAT;
	roughness_texture_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	roughness_texture_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	roughness_texture_desc.sampler = sampler;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
	depth_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	depth_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	depth_desc.sampler = sampler;

	auto texture_color_albedo = RenderResourceManager::Get()->CreateTexture(color_texture_desc);
	auto texture_color_normal = RenderResourceManager::Get()->CreateTexture(color_normal_desc);
	auto texture_depth_stencil = RenderResourceManager::Get()->CreateTexture(depth_desc);
	auto roughness_texture = RenderResourceManager::Get()->CreateTexture(roughness_texture_desc);



	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = { {0,texture_color_albedo},{0,texture_color_normal},{0,roughness_texture} };
	framebuffer_desc.depth_stencil_attachment = { 0,texture_depth_stencil };

#pragma region Render_IDs
#ifdef EDITOR

	RenderTexture2DDescriptor id_desc;
	id_desc.format = TextureFormat::R_UNSIGNED_INT;
	id_desc.height = Application::Get()->GetWindow()->GetProperties().resolution_y;
	id_desc.width = Application::Get()->GetWindow()->GetProperties().resolution_x;
	id_desc.sampler = sampler;
	auto id_buffer = RenderResourceManager::Get()->CreateTexture(id_desc);
	framebuffer_desc.color_attachments.push_back({ 0,id_buffer });
	data->id_texture = framebuffer_desc.color_attachments.size() - 1;
#endif
#pragma endregion


	data->output_buffer_resource = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);

	data->initialized = true;
}


GenerateGBufferPass::GenerateGBufferPass(const std::string& output_buffer) : output_buffer(output_buffer)
{
	data = new internal_data;
	InitPostProcessingPassData();
}

void GenerateGBufferPass::Setup(RenderPassResourceDefinnition& setup_builder)
{
	setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddPersistentResource< std::shared_ptr<RenderFrameBufferResource>>("G_Buffer", data->output_buffer_resource);
#ifdef EDITOR
	setup_builder.AddPersistentResource<int>("ID", data->id_texture);
#endif
}

void GenerateGBufferPass::Render(RenderPipelineResourceManager& resource_manager)
{
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	list->SetRenderTarget(data->output_buffer_resource);
	list->Clear();
	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_buffer, data->output_buffer_resource);
}

GenerateGBufferPass::~GenerateGBufferPass()
{
	if (data) {
		delete data;
	}
}