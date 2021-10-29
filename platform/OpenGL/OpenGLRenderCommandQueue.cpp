#include "OpenGLRenderCommandQueue.h"
#include <Renderer/Renderer.h>
#include <Profiler.h>

OpenGLRenderCommandQueue::OpenGLRenderCommandQueue()
{
	Render_Thread_Object = ThreadManager::GetThreadManager()->GetThread();
	Render_Thread = new std::thread(&OpenGLRenderCommandQueue::RenderLoop,this);
}

void OpenGLRenderCommandQueue::ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists)
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	for (auto list : lists) {
		m_Lists.push(reinterpret_cast<OpenGLRenderCommandList*>(list));
	}
	lock.unlock();
	m_cond_var.notify_one();
}

void OpenGLRenderCommandQueue::ExecuteRenderCommandList(RenderCommandList* list)
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_Lists.push(reinterpret_cast<OpenGLRenderCommandList*>(list));
	lock.unlock();
	m_cond_var.notify_one();
}

//TODO: Use synchronization to make sure everythings finished
OpenGLRenderCommandQueue::~OpenGLRenderCommandQueue()
{
	
	running.store(false);
	auto empty = Renderer::Get()->GetRenderCommandList();
	ExecuteRenderCommandList(empty);
	m_cond_var.notify_one();
	Render_Thread->join();
	delete Render_Thread;
	Render_Thread_Object.reset();
}

void OpenGLRenderCommandQueue::RenderLoop()
{
	PROFILE("RenderLoopStart");
	running.store(true);
	while (running.load()) {
		PROFILE("RenderLoopFetch");
		OpenGLRenderCommandList* list = FetchList();
		if (list) {
			PROFILE("RenderLoopExec");
			list->Execute();
			list->m_Alloc->clear();
			delete list;
		}
	}
}

OpenGLRenderCommandList* OpenGLRenderCommandQueue::FetchList() {
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	if (m_Lists.empty()) {
		PROFILE("Wait");
		m_cond_var.wait(lock, [this]() {return !m_Lists.empty(); });
	}
	OpenGLRenderCommandList* list = m_Lists.front();
	m_Lists.pop();
	return list;
}
