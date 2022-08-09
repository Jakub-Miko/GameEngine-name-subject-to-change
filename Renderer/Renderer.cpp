#include "Renderer.h"
#include <Renderer/Renderer3D/Renderer3D.h>
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#include <Renderer/RenderContext.h>
#include <Renderer/RenderResourceManager.h>
#include <Renderer/ShaderManager.h>
#include <Renderer/PipelineManager.h>
#include <stb_image.h>

Renderer* Renderer::instance = nullptr;

Renderer* Renderer::Get() {
    return instance;
}

void Renderer::PreInit() {
    stbi_set_flip_vertically_on_load(true);
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
    std::unique_lock<std::mutex> lock(m_List_mutex);
    if (m_FreeAllocators.empty()) {
        if (m_Allocators.size() >= max_allocators) {
            m_List_cond.wait(lock, [this]() {return !m_FreeAllocators.empty(); });
            RenderCommandAllocator* reused_alloc = m_FreeAllocators.back();
            m_FreeAllocators.pop_back();
            return reused_alloc;
        }
        else {
            RenderCommandAllocator* new_alloc = RenderCommandAllocator::CreateAllocator(1024);
            m_Allocators.push_back(new_alloc);
            return new_alloc;
        }
    }
    else {
        RenderCommandAllocator* reused_alloc = m_FreeAllocators.back();
        m_FreeAllocators.pop_back();
        return reused_alloc;
    }
}

void Renderer::Init(int max_allocators) {
    this->max_allocators = max_allocators;
    RenderResourceManager::Initialize();
    ShaderManager::Initialize();
    PipelineManager::Initialize();
    if (RenderContext::Get()) {
        RenderContext::Get()->Init();
    }
    Renderer3D::Init();
}


RenderCommandQueue* Renderer::GetCommandQueue(RenderQueueTypes type)
{
    return m_CommandQueues[(unsigned char)type];
}

RenderFence* Renderer::GetFence()
{
    return RenderFence::CreateFence();
}

void Renderer::Shutdown()
{
    Renderer3D::PreShutdown();
    std::unique_lock<std::mutex> lock(instance->default_frame_buffer_mutex);
    if (instance->default_frame_buffer) {
        instance->default_frame_buffer.reset();
    }
    lock.unlock();
    RenderContext::Get()->StartShutdown();
    PipelineManager::Shutdown();
    ShaderManager::Shutdown();
    Renderer3D::Shutdown();
    RenderResourceManager::Shutdown();
    if (instance) {
        instance->Destroy();
        delete instance;
    }
}

void Renderer::SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type)
{
    m_CommandQueues[(unsigned char)type] = queue;
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

void Renderer::Update(float delta_time)
{
    Renderer3D::Get()->Update(delta_time);
}

void Renderer::ReuseAllocator(RenderCommandAllocator* alloc)
{
    alloc->clear();
    std::lock_guard<std::mutex> lock(m_List_mutex);
    m_FreeAllocators.push_back(alloc);
    m_List_cond.notify_one();
}
