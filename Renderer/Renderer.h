#pragma once
#include <Renderer/RenderCommandList.h>
#include <Renderer/RenderFence.h>
#include <condition_variable>
#include <Renderer/RenderCommandAllocator.h>
#include <Renderer/RenderCommandQueue.h>
#include <memory>
#include <vector>
#include <array>
#include <mutex>
#include <Renderer/RendererDefines.h>

struct PerThreadRendererData {
    std::shared_ptr<RenderCommandAllocator> general_allocator;
};

class Renderer {
public: 

    Renderer(const Renderer& ref) = delete;
    Renderer(Renderer&& ref) = delete;
    Renderer& operator=(const Renderer& ref) = delete;
    Renderer& operator=(Renderer&& ref) = delete;
    
    void PreInit();

    std::shared_ptr<RenderCommandList> GetRenderCommandList();

    void Init();

    void PostInit();

    RenderCommandQueue* GetCommandQueue(RenderQueueTypes type = RenderQueueTypes::DirectQueue);

    RenderFence* GetFence();

    static void Shutdown();

    static void Create();

    void Update(float delta_time);

    bool CheckDefaultRenderSurfaceValidity();

    std::shared_ptr<RenderFrameBufferResource> GetDefaultFrameBuffer();

    void SetDefaultFrameBuffer(std::shared_ptr<RenderFrameBufferResource> buffer = nullptr) {
        std::lock_guard<std::mutex> lock(default_frame_buffer_mutex);
        default_frame_buffer = buffer;
    }


private:
    Renderer();
    friend class RenderContext;
    void SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type);
    void Destroy();
    
    std::array<RenderCommandQueue*, 3> m_CommandQueues;

    std::mutex default_frame_buffer_mutex;
    std::shared_ptr<RenderFrameBufferResource> default_frame_buffer = nullptr;

    static Renderer* instance;
public:
    
    static Renderer* Get();
};
