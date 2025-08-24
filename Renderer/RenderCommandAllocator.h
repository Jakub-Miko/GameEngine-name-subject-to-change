#pragma once
#include <cstddef>
#include <Renderer/RenderCommandList.h>

class RenderCommandAllocator {
public:
	virtual void clear() = 0;

	virtual std::shared_ptr<RenderCommandList> GetCommandList() = 0;

	virtual ~RenderCommandAllocator() {}

	static std::shared_ptr<RenderCommandAllocator> CreateAllocator(size_t starting_size);
};