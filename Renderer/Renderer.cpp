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
    return RenderCommandList::CreateQueue(this,GetCommandAllocator());
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

void Renderer::SubmitQueue(RenderCommandList* queue)
{
    std::lock_guard<std::mutex> lock(m_List_mutex);
    m_Lists.push_back(queue);
}

//TODO: Synchronize queue reuse instead of clearing it rigth away!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void Renderer::Render()
{
    std::lock_guard<std::mutex> lock(m_List_mutex);
    for (auto queue : m_Lists) {
        queue->Execute();
        queue->m_Alloc->clear();
        m_FreeAllocators.push_back(queue->m_Alloc);
        delete queue;
    }
    m_Lists.clear();
}

void Renderer::Shutdown()
{
    if (instance) {
        instance->Destroy();
        delete instance;
    }
}

void Renderer::Destroy()
{
    RenderContext::Shutdown();
    for (auto queue : m_Lists) {
        delete queue;
    }
    for (auto alloc : m_Allocators) {
        delete alloc;
    }
    m_Lists.clear();
    m_FreeAllocators.clear();
    m_Allocators.clear();
}

void Renderer::Create()
{
    if (!instance) {
        instance = new Renderer();
    }
}
