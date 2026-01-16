#pragma once
#include "RenderPipeline.h"

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
        return rendering_pipeline->template GetPersistentResource<T>(name);
    }

    std::shared_ptr<RenderPipeline> GetPipeline() { return rendering_pipeline; }

private:
    Renderer3D();
    static Renderer3D* instance;

private:
    std::shared_ptr<RenderPipeline> rendering_pipeline;
};
