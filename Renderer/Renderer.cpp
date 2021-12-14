#include "Renderer.h"
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#include <Renderer/RenderContext.h>

Renderer* Renderer::instance = nullptr;

Renderer* Renderer::Get() {
    return instance;
}

void Renderer::PreInit() {
    RenderContext::Create();
    if(RenderContext::Get()) {
        RenderContext::Get()->PreInit();
    }
}

RenderCommandList* Renderer::GetRenderCommandList()
{
    return RenderCommandList::CreateQueue(this);
}


void Renderer::Init(int max_allocators) {
    this->max_allocators = max_allocators;
    if (RenderContext::Get()) {
        RenderContext::Get()->Init();
    }
}


RenderCommandQueue* Renderer::GetCommandQueue(RenderQueueTypes type)
{
    return m_CommandQueues[type];
}

RenderFence* Renderer::GetFence()
{
    return RenderFence::CreateFence();
}

void Renderer::Shutdown()
{
    if (instance) {
        instance->Destroy();
        delete instance;
    }
}

void Renderer::SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type)
{
    m_CommandQueues[type] = queue;
}

void Renderer::Destroy()
{
    RenderContext::Shutdown();

    for (auto alloc : m_Allocators) {
        delete alloc;
    }
    m_FreeAllocators.clear();
    m_Allocators.clear();
}

void Renderer::Create()
{
    if (!instance) {
        instance = new Renderer();
    }
}

void Renderer::ReuseAllocator(RenderCommandAllocator* alloc)
{
    alloc->clear();
    std::lock_guard<std::mutex> lock(m_List_mutex);
    m_FreeAllocators.push_back(alloc);
    m_List_cond.notify_one();
}
