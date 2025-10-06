#pragma once
#include <memory>

#include "Layer.h"
#include "Renderer/PipelineManager.h"

class OneSweepRadixSort : public Layer {
public:
    OneSweepRadixSort();
    void OnUpdate(float delta_time) override;
private:
    std::shared_ptr<Pipeline> global_histogram_pipeline;
    std::shared_ptr<Pipeline> global_prefix_sum_pipeline;
    std::shared_ptr<Pipeline> digit_binning_pipeline;
    std::shared_ptr<Material> global_histogram_mat;
    std::shared_ptr<Material> global_prefix_sum_mat;
    std::shared_ptr<Material> digit_binning_mat;
    std::shared_ptr<RenderBufferResource> input_buffer;
    std::shared_ptr<RenderBufferResource> alt_buffer;
    std::shared_ptr<RenderBufferResource> global_histogram_buffer;
    std::shared_ptr<RenderBufferResource> global_prefix_sum_buffer;
    std::shared_ptr<RenderBufferResource> digit_binning_buffer;

    std::shared_ptr<RenderBufferResource> validation_buffer;
};
