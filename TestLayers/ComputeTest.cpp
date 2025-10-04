#include "ComputeTest.h"

#include "Renderer/RenderResourceManager.h"

ComputeTestLayer::ComputeTestLayer() {
    MaterialManager::Get()->LoadMaterialTemplateFile("api:layouts/ComputeMatLayout.json");
    ComputePipelineDescriptor descriptor = {};
    descriptor.shader = ShaderManager::Get()->GetShader("shaders/ComputeShader.glsl");
    pipeline = PipelineManager::Get()->CreatePipeline(descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.buffer_size = 1000 * sizeof(int);
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    auto mat_template = MaterialManager::Get()->GetMaterialTemplate("ComputeSetting");
    setting_material = mat_template->CreateMaterial();
    setting_material->SetParameter("buffer_size", 1000);
    setting_material->SetParameter("block_count", 1);
    setting_material->SetParameter("data_buffer", buffer);
}

void ComputeTestLayer::OnUpdate(float delta_time) {
    auto list = Renderer::Get()->GetRenderCommandList();


    list->SetPipeline(pipeline);
    list->SetMaterial("ComputeSetting",setting_material);
    list->Dispatch(1,1,1);


    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
