#include "MultiThreadBlockPrefixSum.h"

#include <iostream>
#include <random>


#include "Renderer/Renderer.h"
#include "Renderer/RenderResourceManager.h"

#define NUM_OF_THREADBLOCKS 10
#define NUM_OF_THREADS_IN_THREADBLOCK 1000
#define NUM_OF_THREADS NUM_OF_THREADBLOCKS * NUM_OF_THREADS_IN_THREADBLOCK

struct StateBlock {
    uint32_t state_flag;
    uint32_t reduction;
    uint32_t prefix;
};

#define STATE_BUFFER_SIZE NUM_OF_THREADBLOCKS * sizeof(StateBlock) + sizeof(uint32_t)

MultiThreadBlockPrefixSum::MultiThreadBlockPrefixSum() {
    ComputePipelineDescriptor descriptor = {};
    descriptor.shader = ShaderManager::Get()->GetShader("compute/MultiThreadblockPrefixSum.glsl");
    pipeline = PipelineManager::Get()->CreatePipeline(descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.buffer_size = NUM_OF_THREADS * sizeof(int);
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    input_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    output_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    validation_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    RenderBufferDescriptor state_buffer_desc = {};
    state_buffer_desc.buffer_size = STATE_BUFFER_SIZE;
    state_buffer_desc.type = RenderBufferType::DEFAULT;
    state_buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    state_buffer = RenderResourceManager::Get()->CreateBuffer(state_buffer_desc);

    auto mat_template = MaterialManager::Get()->GetMaterialTemplate("MultiThreadblockPrefixSumSettings");
    material = mat_template->CreateMaterial();
    material->SetParameter("buffer_size", 1000);
    material->SetParameter("block_count", 10);
    material->SetParameter("data_buffer", input_buffer);
    material->SetParameter("state_buffer", state_buffer);
    material->SetParameter("output_buffer", output_buffer);
    material->SetParameter("validation_buffer", validation_buffer);
}

void MultiThreadBlockPrefixSum::OnUpdate(float delta_time) {
    std::cout << "Running Compute\n";
    auto list = Renderer::Get()->GetRenderCommandList();


    int random_nums[NUM_OF_THREADS] = {};
    StateBlock state_block[NUM_OF_THREADBLOCKS] = {};

    RenderResourceManager::Get()->UploadDataToBuffer(list, output_buffer,&random_nums, sizeof(int) * NUM_OF_THREADS,0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, validation_buffer,&random_nums, sizeof(int) * NUM_OF_THREADS,0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, state_buffer,&random_nums, sizeof(int),0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, state_buffer,&state_block, sizeof(StateBlock) * NUM_OF_THREADBLOCKS,sizeof(int));

    std::random_device generator;
    std::mt19937 rng(generator());
    std::uniform_int_distribution<int> distribution(0,10);
    int prefix_sum[NUM_OF_THREADS] = {};
    int counter = 0;
    for(int i = 0; i < NUM_OF_THREADS; i++) {
        auto num = distribution(rng);
        random_nums[i] = num;
        prefix_sum[i] = counter;
        counter += num;
    }

    RenderResourceManager::Get()->UploadDataToBuffer(list, input_buffer,&random_nums, sizeof(int) * NUM_OF_THREADS,0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, validation_buffer,&prefix_sum, sizeof(int) * NUM_OF_THREADS,0);

    list->SetPipeline(pipeline);
    list->SetMaterial("MultiThreadblockPrefixSumSettings",material);
    list->Dispatch(NUM_OF_THREADBLOCKS,1,1);


    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
