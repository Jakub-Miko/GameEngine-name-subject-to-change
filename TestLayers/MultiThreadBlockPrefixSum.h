#pragma once
#include <memory>

#include "Layer.h"
#include "Renderer/PipelineManager.h"

class MultiThreadBlockPrefixSum : public Layer {
public:
    MultiThreadBlockPrefixSum();
    void OnUpdate(float delta_time) override;
private:
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Material> material;
    std::shared_ptr<RenderBufferResource> input_buffer;
    std::shared_ptr<RenderBufferResource> output_buffer;
    std::shared_ptr<RenderBufferResource> state_buffer;
    std::shared_ptr<RenderBufferResource> validation_buffer;
};
