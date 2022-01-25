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

class OpenGLRenderResourceManager;

class OpenGLRenderCommandQueue : public RenderCommandQueue {
public:
	friend OpenGLRenderResourceManager;
	friend class OpenGLShaderManager;
	friend class OpenGLPipelineManager;
	friend class Renderer;
	friend class OpenGLRenderContext;
	OpenGLRenderCommandQueue();

	OpenGLRenderCommandQueue(const OpenGLRenderCommandQueue& ref) = delete;
	OpenGLRenderCommandQueue& operator=(const OpenGLRenderCommandQueue& ref) = delete;

	OpenGLRenderCommandQueue(OpenGLRenderCommandQueue&& ref) = delete;
	OpenGLRenderCommandQueue& operator=(OpenGLRenderCommandQueue&& ref) = delete;

	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;
	virtual void ExecuteRenderCommandList(RenderCommandList* list) override;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) override;
	virtual void Present() override;

	virtual ~OpenGLRenderCommandQueue();
private:
	void RenderLoop();
	ExecutableCommand* FetchList();

	//When Shutdown starts, the queue stops executing commmands, calls destructors of the remaining ones, the waits and unblocks this function call
	//After this function call the queue is guaranteed to be inactive and finished with its tasks, but destruction tasks which dont use other render systems can 
	//still be submitted.
	//Before the queue gets destroyed(after all other render systems do) it resumes and finishes the task submitted during the renderer destruction phase, then exits.
	void StartShutdownPhase();
	std::mutex shutdown_mutex;
	std::condition_variable shutdown_cond;
	int shutdown_flag = 0;

	void ExecuteCustomCommand(ExecutableCommand* command);

	std::atomic_bool running = false;
	std::mutex m_queue_mutex;
	std::condition_variable m_cond_var;
	std::queue<ExecutableCommand*> m_Lists;
	std::shared_ptr<ThreadObject> Render_Thread_Object;
	std::thread* Render_Thread;
};
