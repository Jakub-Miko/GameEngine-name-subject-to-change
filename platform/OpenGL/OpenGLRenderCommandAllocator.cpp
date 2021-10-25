#include "OpenGLRenderCommandAllocator.h"

OpenGLRenderCommandAllocator::OpenGLRenderCommandAllocator(size_t starting_pool_size) : m_pool(std::allocator<void>(),starting_pool_size)
{

}

OpenGLRenderCommandAllocator::OpenGLRenderCommandAllocator(OpenGLRenderCommandAllocator&& ref) : m_pool(std::move(ref.m_pool))
{

}

OpenGLRenderCommandAllocator& OpenGLRenderCommandAllocator::operator=(OpenGLRenderCommandAllocator&& ref)
{
	m_pool = std::move(ref.m_pool);
	return *this;
}

OpenGLRenderCommandAllocator::~OpenGLRenderCommandAllocator()
{
	m_pool.release();
}

void* OpenGLRenderCommandAllocator::Get()
{
	return &m_pool;
}

void OpenGLRenderCommandAllocator::clear()
{
	m_pool.clear();
}
