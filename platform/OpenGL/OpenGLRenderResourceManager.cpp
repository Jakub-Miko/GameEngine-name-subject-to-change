#include "OpenGLRenderResourceManager.h"
#include <stdexcept>
#include <memory_resource>
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <GL/glew.h>
#include <cstring>

std::shared_ptr<RenderBufferResource> OpenGLRenderResourceManager::CreateBuffer(const RenderBufferDescriptor& buffer_desc)
{
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderBufferResource>(&ResourcePool);
	std::unique_lock<std::mutex> lock(ResourceMutex);
	OpenGLRenderBufferResource* resource = std::allocator_traits<decltype(allocator)>::allocate(allocator, 1);
	lock.unlock();
	std::allocator_traits<decltype(allocator)>::construct(allocator, resource, buffer_desc, RenderState::UNINITIALIZED);
	auto ptr = std::shared_ptr<RenderBufferResource>(static_cast<RenderBufferResource*>(resource), [this](RenderBufferResource* ptr) {
		ReturnBufferResource(ptr);
		});

	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([ptr] {
		if (ptr->GetRenderState() == RenderState::UNINITIALIZED) {
			unsigned int buffer;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);

			//TODO: The usage choice needs to be reworked
			glBufferData(GL_COPY_WRITE_BUFFER, ptr->GetBufferDescriptor().buffer_size, NULL,
				ptr->GetBufferDescriptor().type == RenderBufferType::DEFAULT ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			static_cast<OpenGLRenderBufferResource*>(ptr.get())->SetRenderId(buffer);
			ptr->SetRenderState(RenderState::COMMON);
		}
		else {
			throw std::runtime_error("Resource has already been Initialied");
		}
		}));
	return ptr;
}

void OpenGLRenderResourceManager::UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset)
{
	void* allocated = new char[size];
	memcpy(allocated, data, size);
	static_cast<OpenGLRenderCommandList*>(list)->UpdateBufferResource(resource, allocated, size, offset);
}

OpenGLRenderResourceManager::OpenGLRenderResourceManager() : ResourcePool(std::allocator<void>(),1024), ResourceMutex()
{

}

OpenGLRenderResourceManager::~OpenGLRenderResourceManager()
{
}

void OpenGLRenderResourceManager::ReturnBufferResource(RenderBufferResource* resource)
{
	OpenGLRenderBufferResource* ptr = static_cast<OpenGLRenderBufferResource*>(resource);
	unsigned int vao = 0;
	unsigned int vbo = ptr->render_id;
	vao = ptr->extra_id;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([vao, vbo] {
		glDeleteBuffers(1, &vbo);
		if (vao) {
			glDeleteVertexArrays(1, &vao);
		}
		}));
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderBufferResource>(&ResourcePool);
	std::allocator_traits<decltype(allocator)>::destroy(allocator, ptr);
	std::lock_guard<std::mutex> lock(ResourceMutex);
	std::allocator_traits<decltype(allocator)>::deallocate(allocator,ptr, 1);
}
