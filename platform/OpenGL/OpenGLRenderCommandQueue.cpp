#include "OpenGLRenderCommandQueue.h"
#include <Renderer/Renderer.h>
#include "OpenGLRenderFence.h"
#include <Profiler.h>
#include "OpenGLPresent.h"

OpenGLRenderCommandQueue::OpenGLRenderCommandQueue() : current_state(), m_queue_mutex(), m_cond_var(), m_Lists(), Render_Thread(), Render_Thread_Object()
{
	Render_Thread_Object = ThreadManager::Get()->GetThread();
	Render_Thread = new std::thread(&OpenGLRenderCommandQueue::RenderLoop,this);
}

void OpenGLRenderCommandQueue::ExecuteCommand(ExecutableCommand* command)
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_Lists.push(command);
	lock.unlock();
	m_cond_var.notify_one();
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
	shutdown_flag = 2;
	shutdown_cond.notify_all();
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
		ExecutableCommand* list = FetchList();
		if (list) {
			PROFILE("RenderLoopExec");
			list->Execute();
			delete list;
		}
	}

	while(!m_Lists.empty()) {
		delete m_Lists.front();
		std::unique_lock<std::mutex> lock(m_queue_mutex);
		m_Lists.pop();
	}


	std::unique_lock<std::mutex> shutdown_lock(shutdown_mutex);
	shutdown_flag = 1;
	shutdown_cond.notify_all();
	shutdown_cond.wait(shutdown_lock, [this]() {return shutdown_flag == 2; });

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

void OpenGLRenderCommandQueue::StartShutdownPhase()
{
	running.store(false);
	auto empty = Renderer::Get()->GetRenderCommandList();
	ExecuteRenderCommandList(empty);
	m_cond_var.notify_one();
	std::unique_lock<std::mutex> shutdown_lock(shutdown_mutex);
	shutdown_cond.wait(shutdown_lock, [this]() {return shutdown_flag == 1; });
}

void OpenGLRenderCommandQueue::ExecuteCustomCommand(ExecutableCommand* command)
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_Lists.push(command);
	lock.unlock();
	m_cond_var.notify_one();
}
