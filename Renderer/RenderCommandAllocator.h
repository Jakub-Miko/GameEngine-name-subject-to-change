#pragma once

class RenderCommandAllocator {
public:
	virtual void* Get() = 0;
	virtual void clear() = 0;

	virtual ~RenderCommandAllocator() {}

	static RenderCommandAllocator* CreateAllocator(size_t starting_size);
};