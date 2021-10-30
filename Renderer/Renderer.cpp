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
    return RenderCommandList::CreateQueue(this, std::shared_ptr<RenderCommandAllocator>(GetCommandAllocator(), 
        [this](RenderCommandAllocator* ptr) { ReuseAllocator(ptr); }));
}

RenderCommandAllocator* Renderer::GetCommandAllocator()
{
    if (m_FreeAllocators.empty()) {
        RenderCommandAllocator* new_alloc = RenderCommandAllocator::CreateAllocator(1024);
        m_Allocators.push_back(new_alloc);
        return new_alloc;
    }
    else {
        RenderCommandAllocator* reused_alloc = m_FreeAllocators.back();
        m_FreeAllocators.pop_back();
        return reused_alloc;
    }
}

void Renderer::Init() {
    if (RenderContext::Get()) {
        RenderContext::Get()->Init();
    }
}


RenderCommandQueue* Renderer::GetCommandQueue(RenderQueueTypes type)
{
    return m_CommandQueues[type];
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
    std::lock_guard<std::mutex> lock(m_List_mutex);
    m_FreeAllocators.push_back(alloc);
}
