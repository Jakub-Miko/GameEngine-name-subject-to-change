#pragma once
#include <Renderer/RenderCommandList.h>
#include <vector>

class RenderCommandQueue {
public:
	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) = 0;

	static RenderCommandQueue* CreateRenderCommandQueue();

	~RenderCommandQueue() {};
}; 