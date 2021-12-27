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

enum class RenderPrimitiveType : unsigned char
{
    FLOAT = 0, INT = 1, UNSIGNED_INT = 2, CHAR = 3, UNSIGNED_CHAR = 4
};

enum class RenderQueueTypes : unsigned char
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

    void Init(int max_allocators = 50);

    RenderCommandQueue* GetCommandQueue(RenderQueueTypes type = RenderQueueTypes::DirectQueue);

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
    std::array<RenderCommandQueue*, 50> m_CommandQueues;

    std::mutex m_List_mutex;
    int max_allocators = 50;
    std::condition_variable m_List_cond;

    static Renderer* instance;
public:
    
    static Renderer* Get();
};
