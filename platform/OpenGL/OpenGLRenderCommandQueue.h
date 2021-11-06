#pragma once
#include <atomic>
#include <Renderer/RenderCommandQueue.h>
#include "OpenGLRenderCommandList.h"
#include <ThreadManager.h>
#include <Core/ExecutableCommand.h>
#include <mutex>
#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>

class OpenGLRenderCommandQueue : public RenderCommandQueue {
public:

	OpenGLRenderCommandQueue();

	OpenGLRenderCommandQueue(const OpenGLRenderCommandQueue& ref) = delete;
	OpenGLRenderCommandQueue& operator=(const OpenGLRenderCommandQueue& ref) = delete;

	OpenGLRenderCommandQueue(OpenGLRenderCommandQueue&& ref) = delete;
	OpenGLRenderCommandQueue& operator=(OpenGLRenderCommandQueue&& ref) = delete;

	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;
	virtual void ExecuteRenderCommandList(RenderCommandList* list) override;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) override;


	virtual ~OpenGLRenderCommandQueue();
private:
	void RenderLoop();
	ExecutableCommand* FetchList();
	std::atomic_bool running = false;
	std::mutex m_queue_mutex;
	std::condition_variable m_cond_var;
	std::queue<ExecutableCommand*> m_Lists;
	std::shared_ptr<ThreadObject> Render_Thread_Object;
	std::thread* Render_Thread;
};
