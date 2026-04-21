#include "OneSweepRadixSort.h"

#include <iostream>
#include <random>

#include "Renderer/RenderResourceManager.h"

#define THREADS_PER_THREADBLOCK 256
#define ITEMS 1000000
#define ITEMS_PER_THREAD 16
#define THREAD_BLOCKS 245
#define DIGIT_COUNT 4
//#define VALIDATE_HISTOGRAM
#define VALIDATE_PREFIX_SUM

struct DigitBinningInfo {
    uint32_t thread_block_number[DIGIT_COUNT] = {};
    uint32_t thread_block_data[256][THREAD_BLOCKS] = {};
};

OneSweepRadixSort::OneSweepRadixSort() {
    ComputePipelineDescriptor global_histogram_pipeline_descriptor = {};
    global_histogram_pipeline_descriptor.shader = ShaderManager::Get()->GetShader("compute/radix_sort/GlobalHistogram.glsl");
    global_histogram_pipeline = PipelineManager::Get()->CreatePipeline(global_histogram_pipeline_descriptor);

    ComputePipelineDescriptor global_prefix_sum_pipeline_descriptor = {};
    global_prefix_sum_pipeline_descriptor.shader = ShaderManager::Get()->GetShader("compute/radix_sort/GlobalPrefixSum.glsl");
    global_prefix_sum_pipeline = PipelineManager::Get()->CreatePipeline(global_prefix_sum_pipeline_descriptor);

    ComputePipelineDescriptor digit_binning_pipeline_descriptor = {};
    digit_binning_pipeline_descriptor.shader = ShaderManager::Get()->GetShader("compute/radix_sort/DigitBinning.glsl");
    digit_binning_pipeline = PipelineManager::Get()->CreatePipeline(digit_binning_pipeline_descriptor);

    RenderBufferDescriptor buffer_desc = {};
    buffer_desc.type = RenderBufferType::DEFAULT;
    buffer_desc.usage = RenderBufferUsage::STORAGE_BUFFER;

    buffer_desc.buffer_size = ITEMS * sizeof(uint32_t);
    input_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
    alt_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);
    alt2_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

    buffer_desc.buffer_size = sizeof(DigitBinningInfo);
    digit_binning_buffer = RenderResourceManager::Get()->CreateBuffer(buffer_desc);

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

    mat_template = MaterialManager::Get()->GetMaterialTemplate("DigitBinningSettings");
    digit_binning_mat = mat_template->CreateMaterial();
    digit_binning_mat->SetParameter("input_prefix_sum", global_prefix_sum_buffer);
    digit_binning_mat->SetParameter("input_buffer", input_buffer);
    digit_binning_mat->SetParameter("output_buffer", alt_buffer);
    digit_binning_mat->SetParameter("digit_binning_data", digit_binning_buffer);
    digit_binning_mat->SetParameter("input_size", ITEMS);
    digit_binning_mat->SetParameter("block_count", THREAD_BLOCKS);
    digit_binning_mat->SetParameter("shift", 0);

    auto list = Renderer::Get()->GetRenderCommandList();

    std::random_device seed_gen;
    std::mt19937_64 rng(seed_gen());
    std::uniform_int_distribution<> item_dist(0, 5000);
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

    uint32_t histogram_data[sizeof(DigitBinningInfo)] = {};

    RenderResourceManager::Get()->UploadDataToBuffer(list, global_histogram_buffer, histogram_data, sizeof(uint32_t) * 256 * 4, 0);
    RenderResourceManager::Get()->UploadDataToBuffer(list, digit_binning_buffer, histogram_data, sizeof(DigitBinningInfo), 0);

    list->SetPipeline(global_histogram_pipeline);
    list->SetMaterial("GlobalHistogramSettings", global_histogram_mat);
    list->Dispatch(THREAD_BLOCKS, 1,1);

    list->SetPipeline(global_prefix_sum_pipeline);
    list->SetMaterial("GlobalPrefixSumSettings", global_prefix_sum_mat);
    list->Dispatch(4,1,1);

    list->SetPipeline(digit_binning_pipeline);

    digit_binning_mat->SetParameter("input_buffer", input_buffer);
    digit_binning_mat->SetParameter("output_buffer", alt_buffer);
    digit_binning_mat->SetParameter("shift", 0);
    list->SetMaterial("DigitBinningSettings", digit_binning_mat);
    list->Dispatch(THREAD_BLOCKS,1,1);

    //RenderResourceManager::Get()->UploadDataToBuffer(list, digit_binning_buffer, histogram_data, sizeof(DigitBinningInfo), 0);
    digit_binning_mat->SetParameter("input_buffer", alt_buffer);
    digit_binning_mat->SetParameter("output_buffer", alt2_buffer);
    digit_binning_mat->SetParameter("shift", 8);
    list->SetMaterial("DigitBinningSettings", digit_binning_mat);
    list->Dispatch(THREAD_BLOCKS,1,1);


    // digit_binning_mat->SetParameter("input_buffer", input_buffer);
    // digit_binning_mat->SetParameter("output_buffer", alt_buffer);
    // digit_binning_mat->SetParameter("shift", 16);
    // list->SetMaterial("DigitBinningSettings", digit_binning_mat);
    // list->Dispatch(THREAD_BLOCKS,1,1);
    //
    // digit_binning_mat->SetParameter("input_buffer", alt_buffer);
    // digit_binning_mat->SetParameter("output_buffer", input_buffer);
    // digit_binning_mat->SetParameter("shift", 24);
    // list->SetMaterial("DigitBinningSettings", digit_binning_mat);
    // list->Dispatch(THREAD_BLOCKS,1,1);


    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
