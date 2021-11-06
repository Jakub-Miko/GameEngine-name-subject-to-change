#pragma once
#include <Renderer/RenderCommandList.h>
#include <Renderer/RenderFence.h>
#include <vector>

class RenderCommandQueue {
public:
	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) = 0;
	virtual void ExecuteRenderCommandList(RenderCommandList* list) = 0;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) = 0;

	static RenderCommandQueue* CreateRenderCommandQueue();

	virtual ~RenderCommandQueue() {};
}; 