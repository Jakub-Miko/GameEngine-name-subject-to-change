#pragma once
#include <memory>

#include "RenderPipeline.h"

class ForwardClusteredRendererPipeline {
public:
    static std::shared_ptr<RenderPipeline> CreatePipeline();
};
