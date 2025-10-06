#pragma once
#include <memory>

#include "Layer.h"
#include "Renderer/PipelineManager.h"

struct StateBlock {
    uint32_t state_flag;
};

class MultiThreadBlockPrefixSum : public Layer {
public:
    ~MultiThreadBlockPrefixSum() override;
    MultiThreadBlockPrefixSum();
    void OnUpdate(float delta_time) override;
private:
    std::unique_ptr<int[]> input;
    std::unique_ptr<int[]> output;
    std::unique_ptr<int[]> validate;
    std::unique_ptr<StateBlock[]> state;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Material> material;
    std::shared_ptr<RenderBufferResource> input_buffer;
    std::shared_ptr<RenderBufferResource> output_buffer;
    std::shared_ptr<RenderBufferResource> state_buffer;
    std::shared_ptr<RenderBufferResource> validation_buffer;
};
