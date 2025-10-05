#include "OneThreadBlockPrefixSum.h"

#include <iostream>
#include <random>

#include "Renderer/Renderer.h"
#include "Renderer/RenderResourceManager.h"

OneThreadBlockPrefixSum::OneThreadBlockPrefixSum() {
    ComputePipelineDescriptor descriptor = {};
    descriptor.shader = ShaderManager::Get()->GetShader("compute/OneThreadblockPrefixSum.glsl");
    pipeline = PipelineManager::Get()->CreatePipeline(descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.buffer_size = 1000 * sizeof(int);
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    input_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    output_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    validation_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    auto mat_template = MaterialManager::Get()->GetMaterialTemplate("OneThreadblockPrefixSumSettings");
    material = mat_template->CreateMaterial();
    material->SetParameter("buffer_size", 1000);
    material->SetParameter("block_count", 1);
    material->SetParameter("data_buffer", input_buffer);
    material->SetParameter("output_buffer", output_buffer);
    material->SetParameter("validation_buffer", validation_buffer);
}

void OneThreadBlockPrefixSum::OnUpdate(float delta_time) {
    std::cout << "Running Compute\n";
    auto list = Renderer::Get()->GetRenderCommandList();


    int random_nums[1000] = {};

    RenderResourceManager::Get()->UploadDataToBuffer(list, output_buffer,&random_nums, sizeof(int) * 1000,0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, validation_buffer,&random_nums, sizeof(int) * 1000,0);

    std::random_device generator;
    std::mt19937 rng(generator());
    std::uniform_int_distribution<int> distribution(0,10);
    int prefix_sum[1000] = {};
    int counter = 0;
    for(int i = 0; i < 1000; i++) {
        auto num = distribution(rng);
        random_nums[i] = num;
        prefix_sum[i] = counter;
        counter += num;
    }

    RenderResourceManager::Get()->UploadDataToBuffer(list, input_buffer,&random_nums, sizeof(int) * 1000,0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, validation_buffer,&prefix_sum, sizeof(int) * 1000,0);

    list->SetPipeline(pipeline);
    list->SetMaterial("OneThreadblockPrefixSumSettings",material);
    list->Dispatch(1,1,1);


    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
