#pragma once
#include <memory>

#include "Layer.h"
#include "Renderer/PipelineManager.h"

class OneThreadblockReduction : public Layer {
public:
    OneThreadblockReduction();
    void OnUpdate(float delta_time) override;
private:
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Material> material;
    std::shared_ptr<RenderBufferResource> input_buffer;
    std::shared_ptr<RenderBufferResource> output_buffer;
    std::shared_ptr<RenderBufferResource> validation_buffer;
};
