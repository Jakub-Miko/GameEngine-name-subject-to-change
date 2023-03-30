#pragma once
#include <Renderer/RenderDescriptorHeap.h>
#include "DefferedRenderingPipeline.h"

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

    RenderDescriptorHeap& GetDescriptorHeap() {
        return default_descriptor_heap;
    }

    template<typename T>
    const T& GetPersistentResource(const std::string& name) const {
        return deffered_pipeline->template GetPersistentResource<T>(name);
    }

private:
    Renderer3D();
    static Renderer3D* instance;

private:
    RenderDescriptorHeap default_descriptor_heap;
    std::shared_ptr<RenderPipeline> deffered_pipeline;
};
