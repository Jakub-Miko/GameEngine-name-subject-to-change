#include "OneSweepRadixSort.h"

#include <iostream>
#include <random>

#include "dependencies/OpenAL/fmt-11.1.4/include/fmt/base.h"
#include "Renderer/RenderResourceManager.h"

#define THREADS_PER_THREADBLOCK 256
#define ITEMS 1000000
#define ITEMS_PER_THREAD 16
#define THREAD_BLOCKS 245
//#define VALIDATE_HISTOGRAM
#define VALIDATE_PREFIX_SUM
OneSweepRadixSort::OneSweepRadixSort() {
    ComputePipelineDescriptor global_histogram_pipeline_descriptor = {};
    global_histogram_pipeline_descriptor.shader = ShaderManager::Get()->GetShader("compute/radix_sort/GlobalHistogram.glsl");
    global_histogram_pipeline = PipelineManager::Get()->CreatePipeline(global_histogram_pipeline_descriptor);

    ComputePipelineDescriptor global_prefix_sum_pipeline_descriptor = {};
    global_prefix_sum_pipeline_descriptor.shader = ShaderManager::Get()->GetShader("compute/radix_sort/GlobalPrefixSum.glsl");
    global_prefix_sum_pipeline = PipelineManager::Get()->CreatePipeline(global_prefix_sum_pipeline_descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    buffer_desc.buffer_size = ITEMS * sizeof(uint32_t);
    input_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 4 * 256 * sizeof(uint32_t);
    global_histogram_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = 4 * 256 * sizeof(uint32_t);
    global_prefix_sum_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    auto mat_template = MaterialManager::Get()->GetMaterialTemplate("GlobalHistogramSettings");
    global_histogram_mat = mat_template->CreateMaterial();
    global_histogram_mat->SetParameter("input_buffer", input_buffer);
    global_histogram_mat->SetParameter("histogram_buffer", global_histogram_buffer);
    global_histogram_mat->SetParameter("input_size", ITEMS);
    global_histogram_mat->SetParameter("block_count", THREAD_BLOCKS);

    mat_template = MaterialManager::Get()->GetMaterialTemplate("GlobalPrefixSumSettings");
    global_prefix_sum_mat = mat_template->CreateMaterial();
    global_prefix_sum_mat->SetParameter("input_histogram", global_histogram_buffer);
    global_prefix_sum_mat->SetParameter("output_prefix_sum", global_prefix_sum_buffer);

    auto list = Renderer::Get()->GetRenderCommandList();

    std::random_device seed_gen;
    std::mt19937_64 rng(seed_gen());
    std::uniform_int_distribution<> item_dist(0, 50000);
    std::unique_ptr<uint32_t[]> input_numbers(new uint32_t[ITEMS]());

    for (int i = 0; i < ITEMS; i++) {
        input_numbers[i] = item_dist(rng);
    }

    #ifdef VALIDATE_HISTOGRAM

    struct entry {
        uint32_t digit[4];
    };

    buffer_desc.buffer_size = 256 * 4 * sizeof(uint32_t);
    validation_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
    entry counters[256] = {};
    for(int i = 0; i < ITEMS; i++) {
        for(int x = 0; x < 4; x++) {
            uint32_t digit = (input_numbers[i] >> (8 * x)) & 255;
            counters[digit].digit[x]++;
        }
    }

    RenderResourceManager::Get()->UploadDataToBuffer(list, validation_buffer, &counters, sizeof(uint32_t)* 256 * 4, 0);
    global_histogram_mat->SetParameter("validate_buffer", validation_buffer);

    #endif

    #ifdef VALIDATE_PREFIX_SUM

    struct entry {
        uint32_t digit[4];
    };

    buffer_desc.buffer_size = 256 * 4 * sizeof(uint32_t);
    validation_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
    entry counters[256] = {};
    entry prefix_sum[256] = {};
    for(int i = 0; i < ITEMS; i++) {
        for(int x = 0; x < 4; x++) {
            uint32_t digit = (input_numbers[i] >> (8 * x)) & 255;
            counters[digit].digit[x]++;;
        }
    }

    uint32_t aggregates[4] = {};
    for(int x = 0; x < 256; x++) {
        for(int y = 0; y < 4; y++) {
            prefix_sum[x].digit[y] = aggregates[y];
            aggregates[y] += counters[x].digit[y];
        }
    }

    RenderResourceManager::Get()->UploadDataToBuffer(list, validation_buffer, &prefix_sum, sizeof(uint32_t)* 256 * 4, 0);
    global_prefix_sum_mat->SetParameter("validate_buffer", validation_buffer);

    #endif

    RenderResourceManager::Get()->UploadDataToBuffer(list, input_buffer, input_numbers.get(), sizeof(uint32_t)* ITEMS, 0);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}

void OneSweepRadixSort::OnUpdate(float delta_time) {
    std::cout << "Compute runnning" << std::endl;
    auto list = Renderer::Get()->GetRenderCommandList();

    uint32_t histogram_data[256 * 4] = {};

    RenderResourceManager::Get()->UploadDataToBuffer(list, global_histogram_buffer, histogram_data, sizeof(uint32_t) * 256 * 4, 0);

    list->SetPipeline(global_histogram_pipeline);
    list->SetMaterial("GlobalHistogramSettings", global_histogram_mat);
    list->Dispatch(THREAD_BLOCKS, 1,1);

    list->SetPipeline(global_prefix_sum_pipeline);
    list->SetMaterial("GlobalPrefixSumSettings", global_prefix_sum_mat);
    list->Dispatch(4,1,1);

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
