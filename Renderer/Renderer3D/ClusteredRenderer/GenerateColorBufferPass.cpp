#include "GenerateColorBufferPass.h"

#include "Application.h"
#include "Window.h"
#include "Renderer/RenderResourceManager.h"

GenerateColorBufferPass::GenerateColorBufferPass(const std::string& output_color_framebuffer) : output_color_framebuffer(output_color_framebuffer){
	GenerateColorBuffer();
}

void GenerateColorBufferPass::Setup(RenderPassResourceDefinnition& setup_builder) {
    setup_builder.AddResource<std::shared_ptr<RenderFrameBufferResource>>(output_color_framebuffer, RenderPassResourceDescriptor_Access::WRITE);
	setup_builder.AddPersistentResource< std::shared_ptr<RenderFrameBufferResource>>("Output_Buffer", output_color_buffer);
#ifdef EDITOR
	setup_builder.AddPersistentResource<int>("ID", 1);
#endif
}

void GenerateColorBufferPass::Render(RenderPipelineResourceManager& resource_manager) {
	PROFILE("GBufferSetupPass");
	auto queue = Renderer::Get()->GetCommandQueue();
	auto list = Renderer::Get()->GetRenderCommandList();
	list->SetRenderTarget(output_color_buffer);
	list->Clear();
	queue->ExecuteRenderCommandList(list);
	resource_manager.SetResource<std::shared_ptr<RenderFrameBufferResource>>(output_color_framebuffer, output_color_buffer);
}

void GenerateColorBufferPass::GenerateColorBuffer() {
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

	auto res = Renderer3D::Get()->GetRenderResolution();
	RenderTexture2DDescriptor color_texture_desc;
	color_texture_desc.format = TextureFormat::RGBA_16FLOAT;
	color_texture_desc.usage = TextureUsage::COLOR_ATTACHMENT_READABLE;
	color_texture_desc.height = res.x;
	color_texture_desc.width = res.y;
	color_texture_desc.sampler = sampler;

	RenderTexture2DDescriptor depth_desc;
	depth_desc.format = TextureFormat::DEFAULT_DEPTH;
	depth_desc.usage = TextureUsage::DEPTH_ATTACHMENT_READABLE;
	depth_desc.height = res.x;
	depth_desc.width = res.y;
	depth_desc.sampler = sampler;

	auto texture_color = RenderResourceManager::Get()->CreateTexture(color_texture_desc);
	auto texture_depth_stencil = RenderResourceManager::Get()->CreateTexture(depth_desc);

	RenderFrameBufferDescriptor framebuffer_desc;
	framebuffer_desc.color_attachments = { {0,texture_color} };
	framebuffer_desc.depth_stencil_attachment = { 0,texture_depth_stencil };
#ifdef EDITOR
	RenderTexture2DDescriptor id_desc;
	id_desc.format = TextureFormat::R_UNSIGNED_INT;
	id_desc.height = res.x;
	id_desc.width = res.y;
	id_desc.usage = TextureUsage::COLOR_ATTACHMENT_READABLE | TextureUsage::COPYABLE;
	id_desc.sampler = sampler;
	auto id_buffer = RenderResourceManager::Get()->CreateTexture(id_desc);
	framebuffer_desc.color_attachments.push_back({ 0,id_buffer });
#endif


	output_color_buffer = RenderResourceManager::Get()->CreateFrameBuffer(framebuffer_desc);
}
