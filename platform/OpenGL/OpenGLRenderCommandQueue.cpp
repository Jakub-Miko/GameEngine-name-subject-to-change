#include "OpenGLRenderCommandQueue.h"
#include <Renderer/Renderer.h>
#include "OpenGLRenderFence.h"
#include <Profiler.h>
#include "OpenGLPresent.h"

OpenGLRenderCommandQueue::OpenGLRenderCommandQueue()
{
	//Render_Thread_Object = ThreadManager::Get()->GetThread();
	Render_Thread = new std::thread(&OpenGLRenderCommandQueue::RenderLoop,this);
}

void OpenGLRenderCommandQueue::ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists)
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	for (auto list : lists) {
		m_Lists.push(reinterpret_cast<ExecutableCommand*>(list));
	}
	lock.unlock();
	m_cond_var.notify_one();
}

void OpenGLRenderCommandQueue::ExecuteRenderCommandList(RenderCommandList* list)
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_Lists.push(reinterpret_cast<ExecutableCommand*>(list));
	lock.unlock();
	m_cond_var.notify_one();
}

void OpenGLRenderCommandQueue::Signal(std::shared_ptr<RenderFence> fence, int num)
{
	ExecutableCommand* command = new OpenGLRenderFenceCommand(fence, num);
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_Lists.push(command);
	lock.unlock();
	m_cond_var.notify_one();
}

void OpenGLRenderCommandQueue::Present()
{
	ExecutableCommand* command = new OpenGLPresent();
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_Lists.push(command);
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
}

void OpenGLRenderCommandQueue::RenderLoop()
{
	PROFILE("RenderLoopStart");
	running.store(true);
	while (running.load()) {
		PROFILE("RenderLoopFetch");
		ExecutableCommand* list = FetchList();
		if (list) {
			PROFILE("RenderLoopExec");
			list->Execute();
			delete list;
		}
	}
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	while(!m_Lists.empty()) {
		delete m_Lists.front();
		m_Lists.pop();
	}
}

ExecutableCommand* OpenGLRenderCommandQueue::FetchList() {
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	if (m_Lists.empty()) {
		PROFILE("Wait");
		m_cond_var.wait(lock, [this]() {return !m_Lists.empty(); });
	}
	ExecutableCommand* list = m_Lists.front();
	m_Lists.pop();
	return list;
}
