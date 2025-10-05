#include "ComputeTest.h"

#include <iostream>
#include <random>
#include "Renderer/RenderResourceManager.h"

ComputeTestLayer::ComputeTestLayer() {
    ComputePipelineDescriptor descriptor = {};
    descriptor.shader = ShaderManager::Get()->GetShader("compute/OneThreadblockStreamCompaction.glsl");
    pipeline = PipelineManager::Get()->CreatePipeline(descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.buffer_size = 1000 * sizeof(int);
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer2 = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
    buffer3 = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    auto mat_template = MaterialManager::Get()->GetMaterialTemplate("OneThreadblockStreamCompactionSettings");
    setting_material = mat_template->CreateMaterial();
    setting_material->SetParameter("buffer_size", 1000);
    setting_material->SetParameter("block_count", 1);
    setting_material->SetParameter("key_lookup", 3);
    setting_material->SetParameter("data_buffer", buffer);
    setting_material->SetParameter("output_buffer", buffer2);


}

void ComputeTestLayer::OnUpdate(float delta_time) {
    std::cout << "Running Compute\n";
    auto list = Renderer::Get()->GetRenderCommandList();


    int random_nums[1000] = {};

    RenderResourceManager::Get()->UploadDataToBuffer(list, buffer2,&random_nums, sizeof(int) * 1000,0);

    std::random_device generator;
    std::uniform_int_distribution<int> distribution(0,1000);
    for(int i = 0; i < 1000; i++) {
        random_nums[i] = distribution(generator);
    }

    RenderResourceManager::Get()->UploadDataToBuffer(list, buffer,&random_nums, sizeof(int) * 1000,0);

    list->SetPipeline(pipeline);
    list->SetMaterial("OneThreadblockStreamCompactionSettings",setting_material);
    list->Dispatch(1,1,1);


    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
