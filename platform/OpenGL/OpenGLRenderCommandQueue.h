#pragma once
#include <Renderer/RenderCommandQueue.h>

class OpenGLRenderCommandQueue : public RenderCommandQueue {
public:

	OpenGLRenderCommandQueue();

	OpenGLRenderCommandQueue(const OpenGLRenderCommandQueue& ref) = delete;
	OpenGLRenderCommandQueue& operator=(const OpenGLRenderCommandQueue& ref) = delete;

	OpenGLRenderCommandQueue(OpenGLRenderCommandQueue&& ref) noexcept;
	OpenGLRenderCommandQueue& operator=(OpenGLRenderCommandQueue&& ref) noexcept;

	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) override;

private:
	void RenderLoop();

};