#pragma once
#include <Renderer/RenderFence.h>
#include <memory>
#include <vector>

class RenderCommandList;

class RenderCommandQueue {
public:
	virtual void ExecuteRenderCommandLists(std::vector<RenderCommandList*>& lists) = 0;
	virtual void ExecuteRenderCommandList(std::shared_ptr<RenderCommandList> list) = 0;

	virtual void Signal(std::shared_ptr<RenderFence> fence, int num) = 0;

	virtual ~RenderCommandQueue() {};
}; 