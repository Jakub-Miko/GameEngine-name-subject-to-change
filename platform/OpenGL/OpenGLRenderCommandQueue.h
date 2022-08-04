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

	void ExecuteCommand(ExecutableCommand* command);

	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;
	virtual void ExecuteRenderCommandList(RenderCommandList* list) override;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) override;
	virtual void Present() override;

	virtual ~OpenGLRenderCommandQueue();

	const PipelineState& GetPipelineState() const {
		return current_state;
	}

	void SetScissorRect(const RenderScissorRect& ref) {
		current_state.scissor_rect = ref;
	}

	void SetViewport(const RenderViewport& ref) {
		current_state.viewport = ref;
	}

	void SetFlags(const PipelineFlags& ref) {
		current_state.flags = ref;
	}

	void SetShader(std::shared_ptr<Shader> shader) {
		current_state.shader = shader;
	}

	void SetBlendFunctions(const PipelineBlendFunctions& blend_functions) {
		current_state.blend_functions = blend_functions;
	}

	void SetPrimitivePolygonRenderMode(const PrimitivePolygonRenderMode& mode) {
		current_state.polygon_render_mode = mode;
	}

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

	PipelineState current_state;

};
