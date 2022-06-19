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

class Renderer {
public: 

    Renderer(const Renderer& ref) = delete;
    Renderer(Renderer&& ref) = delete;
    Renderer& operator=(const Renderer& ref) = delete;
    Renderer& operator=(Renderer&& ref) = delete;
    
    void PreInit();

    RenderCommandList* GetRenderCommandList();

    RenderCommandAllocator* GetCommandAllocator();

    void Init(int max_allocators = 50);

    RenderCommandQueue* GetCommandQueue(RenderQueueTypes type = RenderQueueTypes::DirectQueue);

    RenderFence* GetFence();

    static void Shutdown();

    static void Create();

    std::shared_ptr<RenderFrameBufferResource> GetDefaultFrameBuffer() {
        std::lock_guard<std::mutex> lock(default_frame_buffer_mutex);
        return default_frame_buffer;
    }

    void SetDefaultFrameBuffer(std::shared_ptr<RenderFrameBufferResource> buffer = nullptr) {
        std::lock_guard<std::mutex> lock(default_frame_buffer_mutex);
        default_frame_buffer = buffer;
    }

    void ReuseAllocator(RenderCommandAllocator* alloc);

private:
    friend class RenderContext;
    Renderer() = default;
    void SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type);
    void Destroy();
    
    std::vector<RenderCommandAllocator*> m_Allocators;
    std::vector<RenderCommandAllocator*> m_FreeAllocators;
    std::array<RenderCommandQueue*, 3> m_CommandQueues;

    std::mutex default_frame_buffer_mutex;
    std::shared_ptr<RenderFrameBufferResource> default_frame_buffer = nullptr;

    std::mutex m_List_mutex;
    int max_allocators = 50;
    std::condition_variable m_List_cond;

    static Renderer* instance;
public:
    
    static Renderer* Get();
};
