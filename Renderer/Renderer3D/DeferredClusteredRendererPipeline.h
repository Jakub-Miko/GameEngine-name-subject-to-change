#pragma once
#include <memory>

#include "RenderPipeline.h"

class DeferredClusteredRendererPipeline {
public:
    static std::shared_ptr<RenderPipeline> CreatePipeline();
};
