#include "OpenGLRenderFence.h"
#include <Profiler.h>
#include <GL/glew.h>

bool OpenGLRenderFence::WaitForValue(int desired_value)
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	if (m_Counter >= desired_value) {
		return true;
	}
	else {
		m_Cond.wait(lock, [this, desired_value]() {
			return m_Counter >= desired_value; 
			});
		return true;
	}
}

void OpenGLRenderFence::Wait()
{
	int desired = GetValue();
	Signal(desired);
	WaitForValue(desired);
}

int OpenGLRenderFence::GetValue()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_Counter;
}

OpenGLRenderFence::OpenGLRenderFence() : m_Counter(0), m_Cond(), m_Mutex()
{

}

OpenGLRenderFence::~OpenGLRenderFence()
{

}

void OpenGLRenderFence::Signal(int num)
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_Counter = num;
	lock.unlock();
	m_Cond.notify_all();
}

OpenGLRenderFenceCommand::OpenGLRenderFenceCommand(std::shared_ptr<RenderFence>& fence, int num) : m_fence(fence), num(num)
{

}

void OpenGLRenderFenceCommand::Execute()
{
	PROFILE("Wait");
	glFinish();
	reinterpret_cast<OpenGLRenderFence*>(m_fence.get())->Signal(num);
}
