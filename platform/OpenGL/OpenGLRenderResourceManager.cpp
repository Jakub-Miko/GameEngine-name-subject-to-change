#include "OpenGLRenderResourceManager.h"
#include <stdexcept>
#include <memory_resource>
#include <platform/OpenGL/OpenGLUnitConverter.h>
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
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

std::shared_ptr<RenderTexture2DResource> OpenGLRenderResourceManager::CreateTexture(const RenderTexture2DDescriptor& buffer_desc)
{
	if (!buffer_desc.sampler) {
		throw std::runtime_error("Sampler wasn't supplied to the texture");
	}
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderTexture2DResource>(&ResourcePool);
	std::unique_lock<std::mutex> lock(ResourceMutex);
	OpenGLRenderTexture2DResource* resource = std::allocator_traits<decltype(allocator)>::allocate(allocator, 1);
	lock.unlock();
	std::allocator_traits<decltype(allocator)>::construct(allocator, resource, buffer_desc, RenderState::UNINITIALIZED);
	auto ptr = std::shared_ptr<RenderTexture2DResource>(static_cast<RenderTexture2DResource*>(resource), [this](RenderTexture2DResource* ptr) {
		ReturnTexture2DResource(ptr);
		});

	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([ptr] {
		if (ptr->GetRenderState() == RenderState::UNINITIALIZED) {
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			//TODO: The usage choice needs to be reworked
			glTexImage2D(GL_TEXTURE_2D, 0, OpenGLUnitConverter::TextureFormatToGLInternalformat(ptr->GetBufferDescriptor().format), ptr->GetBufferDescriptor().width,
				ptr->GetBufferDescriptor().height,0, OpenGLUnitConverter::TextureFormatToGLInternalformat(ptr->GetBufferDescriptor().format),
				OpenGLUnitConverter::TextureFormatToGLDataType(ptr->GetBufferDescriptor().format),NULL);

			const TextureSamplerDescritor& sampler = ptr->GetBufferDescriptor().sampler->GetDescriptor();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_U));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_V));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_W));
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(sampler.border_color));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGLUnitConverter::TextureFilterToGLFilter(sampler.filter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGLUnitConverter::TextureFilterToGLFilter(sampler.filter));


			glBindTexture(GL_TEXTURE_2D, 0);

			static_cast<OpenGLRenderTexture2DResource*>(ptr.get())->SetRenderId(texture);
			ptr->SetRenderState(RenderState::COMMON);
		}
		else {
			throw std::runtime_error("Resource has already been Initialied");
		}
		}));
	return ptr;
}

void OpenGLRenderResourceManager::UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y)
{
	size_t size = OpenGLUnitConverter::TextureFormatToTexelSize(resource->GetBufferDescriptor().format) * width * height;
	void* allocated = new char[size];
	memcpy(allocated, data, size);
	static_cast<OpenGLRenderCommandList*>(list)->UpdateTexture2DResource(resource,allocated,width,height,offset_x,offset_y);
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

void OpenGLRenderResourceManager::ReturnTexture2DResource(RenderTexture2DResource* resource)
{
	OpenGLRenderTexture2DResource* ptr = static_cast<OpenGLRenderTexture2DResource*>(resource);
	unsigned int texture = ptr->render_id;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([texture] {
		glDeleteTextures(1, &texture);
		}));
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderTexture2DResource>(&ResourcePool);
	std::allocator_traits<decltype(allocator)>::destroy(allocator, ptr);
	std::lock_guard<std::mutex> lock(ResourceMutex);
	std::allocator_traits<decltype(allocator)>::deallocate(allocator, ptr, 1);
}
