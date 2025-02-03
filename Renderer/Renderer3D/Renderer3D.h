#pragma once
#include <Renderer/RenderDescriptorHeap.h>
#include "DeferredRenderingPipeline.h"

class Renderer3D {
public:

    Renderer3D(const Renderer3D& ref) = delete;
    Renderer3D(Renderer3D&& ref) = delete;
    Renderer3D& operator=(const Renderer3D& ref) = delete;
    Renderer3D& operator=(Renderer3D&& ref) = delete;

    static void Init();
    static void Shutdown();
    static void PreShutdown();
    static Renderer3D* Get();

    void Update(float delta_time);

    template<typename T>
    const T& GetPersistentResource(const std::string& name) const {
        return deferred_pipeline->template GetPersistentResource<T>(name);
    }

private:
    Renderer3D();
    static Renderer3D* instance;

private:
    std::shared_ptr<RenderPipeline> deferred_pipeline;
};
