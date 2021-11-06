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

enum RenderQueueTypes : unsigned char
{
    DirectQueue = 0, ComputeQueue = 1, CopyQueue = 2
};


class Renderer {
public: 

    Renderer(const Renderer& ref) = delete;
    Renderer(Renderer&& ref) = delete;
    Renderer& operator=(const Renderer& ref) = delete;
    Renderer& operator=(Renderer&& ref) = delete;
    
    void PreInit();

    RenderCommandList* GetRenderCommandList();

    RenderCommandAllocator* GetCommandAllocator();

    void Init(int max_allocators = 5);

    RenderCommandQueue* GetCommandQueue(RenderQueueTypes type = DirectQueue);

    RenderFence* GetFence();

    static void Shutdown();

    static void Create();

    void ReuseAllocator(RenderCommandAllocator* alloc);

private:
    friend class RenderContext;
    Renderer() = default;
    void SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type);
    void Destroy();
    
    std::vector<RenderCommandAllocator*> m_Allocators;
    std::vector<RenderCommandAllocator*> m_FreeAllocators;
    std::array<RenderCommandQueue*, 5> m_CommandQueues;

    std::mutex m_List_mutex;
    int max_allocators = 3;
    std::condition_variable m_List_cond;

    static Renderer* instance;
public:
    
    static Renderer* Get();
};
