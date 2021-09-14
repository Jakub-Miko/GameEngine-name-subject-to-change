#include "Renderer.h"
#include <platform/OpenGL/OpenGLRenderQueue.h>
#include <Renderer/RenderContext.h>

Renderer* Renderer::CreateRenderer() {
    return new Renderer();
}

void Renderer::PreInit() {
    if(RenderContext::Get()) {
        RenderContext::Get()->PreInit();
    }
}

RenderQueue* Renderer::GetRenderQueue()
{
    return new OpenGLRenderQueue(shared_from_this());
}

void Renderer::Init() {
    if (RenderContext::Get()) {
        RenderContext::Get()->Init();
    }
}

void Renderer::SubmitQueue(RenderQueue* queue)
{
    std::lock_guard<std::mutex> lock(m_Queue_mutex);
    m_Queues.push_back(queue);
}

void Renderer::Render()
{
    std::lock_guard<std::mutex> lock(m_Queue_mutex);
    for (auto queue : m_Queues) {
        queue->Execute();
        delete queue;
    }
    m_Queues.clear();
}

Renderer::~Renderer()
{
    delete RenderContext::Get();
    for (auto queue : m_Queues) {
        delete queue;
    }
    m_Queues.clear();
}
