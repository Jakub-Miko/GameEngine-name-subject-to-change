#pragma once
#include <memory>

#include "RenderPipeline.h"

class ClusteredRendererPipeline {
public:
    static std::shared_ptr<RenderPipeline> CreatePipeline();
};
