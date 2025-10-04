#pragma once
#include <memory>

#include "Layer.h"
#include "Renderer/PipelineManager.h"

class ComputeTestLayer : public Layer {
public:
    ComputeTestLayer();
    void OnUpdate(float delta_time) override;
private:
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Pipeline> pipeline2;
    std::shared_ptr<RenderBufferResource> buffer;
    std::shared_ptr<RenderBufferResource> buffer2;
    std::shared_ptr<Material> setting_material;
};
