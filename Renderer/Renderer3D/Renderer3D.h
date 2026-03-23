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
        return current_pipeline.second->template GetPersistentResource<T>(name);
    }

    std::shared_ptr<RenderPipeline> GetPipeline() { return current_pipeline.second; }

    void RegisterPipeline(const std::string& name, std::shared_ptr<RenderPipeline> pipeline);

    void SetActivePipeline(const std::string& name);

    void RemovePipeline(const std::string& name);

    const std::unordered_map<std::string, std::shared_ptr<RenderPipeline>>& GetPipelines() { return rendering_pipelines; }

    std::pair<std::string,std::shared_ptr<RenderPipeline>>& GetCurrentPipeline() { return current_pipeline; }

private:
    Renderer3D();
    static Renderer3D* instance;

private:
    std::unordered_map<std::string, std::shared_ptr<RenderPipeline>> rendering_pipelines;
    std::pair<std::string,std::shared_ptr<RenderPipeline>> current_pipeline;
};
