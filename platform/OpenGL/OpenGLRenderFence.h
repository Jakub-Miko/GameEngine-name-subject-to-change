#pragma once
#include <Renderer/RenderFence.h>
#include <mutex>
#include <cstddef>
#include <Core/ExecutableCommand.h>
#include <condition_variable>

class OpenGLRenderFence : public RenderFence {
public:
	friend class OpenGLRenderCommandQueue;
	friend class OpenGLRenderFenceCommand;
	friend class RenderFence;
	virtual bool WaitForValue(int desired_value) override;
	virtual void Wait() override;
	virtual int GetValue() override;
private:
	OpenGLRenderFence();
	virtual ~OpenGLRenderFence();

	OpenGLRenderFence(const OpenGLRenderFence& ref) = delete;
	OpenGLRenderFence(OpenGLRenderFence&& ref) = delete;
	OpenGLRenderFence& operator=(const OpenGLRenderFence& ref) = delete;
	OpenGLRenderFence& operator=(OpenGLRenderFence&& ref) = delete;

	void Signal(int num);
private:
	std::mutex m_Mutex;
	std::condition_variable m_Cond;
	uint32_t m_Counter;
};

class OpenGLRenderFenceCommand : public ExecutableCommand {
public:
	OpenGLRenderFenceCommand(std::shared_ptr<RenderFence>& fence,int num);

	virtual void Execute() override;

private:
	std::shared_ptr<RenderFence> m_fence;
	int num;

};