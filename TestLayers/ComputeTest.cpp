#include "ComputeTest.h"

#include "Renderer/RenderResourceManager.h"

ComputeTestLayer::ComputeTestLayer() {
    ComputePipelineDescriptor descriptor = {};
    descriptor.shader = ShaderManager::Get()->GetShader("shaders/ComputeShader.glsl");
    pipeline = PipelineManager::Get()->CreatePipeline(descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.buffer_size = 1000 * sizeof(int);
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
}

void ComputeTestLayer::OnUpdate(float delta_time) {
    auto list = Renderer::Get()->GetRenderCommandList();

    list->SetPipeline(pipeline);
    list->SetStorageBuffer("buffer", buffer);
    list->Dispatch(1,1,1);
    list->Dispatch(1,1,1);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
