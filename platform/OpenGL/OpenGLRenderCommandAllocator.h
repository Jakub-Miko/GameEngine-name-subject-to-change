#pragma once 
#include <Renderer/RenderCommandAllocator.h>
#include <MultiPool.h>
#include <memory>

class OpenGLRenderCommandAllocator : public RenderCommandAllocator {
public: 

	using Pool = MultiPool<std::allocator<void>>;

	OpenGLRenderCommandAllocator(size_t starting_pool_size = 1024);

	OpenGLRenderCommandAllocator(const OpenGLRenderCommandAllocator& ref) = delete;
	OpenGLRenderCommandAllocator& operator=(const OpenGLRenderCommandAllocator& ref) = delete;

	OpenGLRenderCommandAllocator(OpenGLRenderCommandAllocator&& ref);
	OpenGLRenderCommandAllocator& operator=(OpenGLRenderCommandAllocator&& ref);

	virtual ~OpenGLRenderCommandAllocator();

	virtual void* Get() override;

	virtual void clear() override;

private:
	Pool m_pool;
};