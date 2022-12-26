#include "OpenGLRenderResourceManager.h"
#include <stdexcept>
#include <memory_resource>
#include <platform/OpenGL/OpenGLUnitConverter.h>
#include <Renderer/Renderer.h>
#include <FileManager.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLRenderDescriptorHeapBlock.h>
#include <stb_image.h>
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

void OpenGLRenderResourceManager::ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size)
{
	void* allocated = new char[size];
	memcpy(allocated, data, size);
	static_cast<OpenGLRenderCommandList*>(list)->UpdateBufferResourceAndReallocate(resource, allocated, size);
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
				ptr->GetBufferDescriptor().height,0, OpenGLUnitConverter::TextureFormatToGLFormat(ptr->GetBufferDescriptor().format),
				OpenGLUnitConverter::TextureFormatToGLDataType(ptr->GetBufferDescriptor().format),NULL);

			const TextureSamplerDescritor& sampler = ptr->GetBufferDescriptor().sampler->GetDescriptor();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_U));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_V));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_W));
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(sampler.border_color));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGLUnitConverter::TextureFilterToGLMinFilter(sampler.filter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGLUnitConverter::TextureFilterToGLMagFilter(sampler.filter));


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

void OpenGLRenderResourceManager::UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
	size_t offset_x, size_t offset_y, int level)
{
	size_t size = OpenGLUnitConverter::TextureFormatToTexelSize(resource->GetBufferDescriptor().format) * width * height;
	char* allocated = new char[size];
	memcpy(allocated, data, size);
	static_cast<OpenGLRenderCommandList*>(list)->UpdateTexture2DResource(resource, level, allocated,width,height,offset_x,offset_y);
}

void OpenGLRenderResourceManager::GenerateMIPs(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource)
{
	list->GenerateMIPs(resource);
}

void OpenGLRenderResourceManager::UploadToTexture2DFromFile(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level)
{
	int x, y, channel;
	unsigned char* data = stbi_load(FileManager::Get()->GetAssetFilePath(filepath).c_str(), &x, &y, &channel, 4);
	if (data) {
		static_cast<OpenGLRenderCommandList*>(list)->UpdateTexture2DResource(resource, level, data, x, y, 0, 0);
	}
	else {
		throw std::runtime_error("Image could not be loaded");
	}
}

std::shared_ptr<RenderTexture2DResource> OpenGLRenderResourceManager::CreateTextureFromFile(RenderCommandList* list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler)
{
	int x, y, channel;
	unsigned char* data = stbi_load(FileManager::Get()->GetAssetFilePath(filepath).c_str(), &x, &y, &channel, 4);
	if(data) {
		RenderTexture2DDescriptor desc;
		desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
		desc.height = y;
		desc.width = x;
		desc.sampler = sampler;

		std::shared_ptr<RenderTexture2DResource> texture = CreateTexture(desc);

		UploadDataToTexture2D(list, texture, data, x, y, 0, 0);

		stbi_image_free(data);

		return texture;

	}
	else {
		throw std::runtime_error("Image could not be loaded");
	}
}

std::shared_ptr<RenderTexture2DArrayResource> OpenGLRenderResourceManager::CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc)
{
	if (!buffer_desc.sampler) {
		throw std::runtime_error("Sampler wasn't supplied to the texture");
	}
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderTexture2DArrayResource>(&ResourcePool);
	std::unique_lock<std::mutex> lock(ResourceMutex);
	OpenGLRenderTexture2DArrayResource* resource = std::allocator_traits<decltype(allocator)>::allocate(allocator, 1);
	lock.unlock();
	std::allocator_traits<decltype(allocator)>::construct(allocator, resource, buffer_desc, RenderState::UNINITIALIZED);
	auto ptr = std::shared_ptr<RenderTexture2DArrayResource>(static_cast<RenderTexture2DArrayResource*>(resource), [this](RenderTexture2DArrayResource* ptr) {
		ReturnTexture2DArrayResource(ptr);
		});

	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([ptr] {
		if (ptr->GetRenderState() == RenderState::UNINITIALIZED) {
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			//TODO: The usage choice needs to be reworked
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, OpenGLUnitConverter::TextureFormatToGLInternalformat(ptr->GetBufferDescriptor().format), ptr->GetBufferDescriptor().width,
				ptr->GetBufferDescriptor().height, ptr->GetBufferDescriptor().num_of_textures ,0, OpenGLUnitConverter::TextureFormatToGLFormat(ptr->GetBufferDescriptor().format),
				OpenGLUnitConverter::TextureFormatToGLDataType(ptr->GetBufferDescriptor().format), NULL);

			const TextureSamplerDescritor& sampler = ptr->GetBufferDescriptor().sampler->GetDescriptor();

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_U));
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_V));
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_W));
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(sampler.border_color));
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, OpenGLUnitConverter::TextureFilterToGLMinFilter(sampler.filter));
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, OpenGLUnitConverter::TextureFilterToGLMagFilter(sampler.filter));


			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			static_cast<OpenGLRenderTexture2DArrayResource*>(ptr.get())->SetRenderId(texture);
			ptr->SetRenderState(RenderState::COMMON);
		}
		else {
			throw std::runtime_error("Resource has already been Initialied");
		}
		}));
	return ptr;
}

std::shared_ptr<RenderFrameBufferResource> OpenGLRenderResourceManager::CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc)
{
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderFrameBufferResource>(&ResourcePool);
	std::unique_lock<std::mutex> lock(ResourceMutex);
	OpenGLRenderFrameBufferResource* resource = std::allocator_traits<decltype(allocator)>::allocate(allocator, 1);
	lock.unlock();
	std::allocator_traits<decltype(allocator)>::construct(allocator, resource, buffer_desc, RenderState::UNINITIALIZED);
	auto ptr = std::shared_ptr<RenderFrameBufferResource>(static_cast<RenderFrameBufferResource*>(resource), [this](RenderFrameBufferResource* ptr) {
		ReturnFrameBufferResource(ptr);
		});

	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([ptr] {
		if (ptr->GetRenderState() == RenderState::UNINITIALIZED) {
			unsigned int buffer;
			glGenFramebuffers(1, &buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, buffer);
			std::vector<GLenum> attachments;
			for (int i = 0; i < ptr->GetBufferDescriptor().color_attachments.size();i++) {
				unsigned int render_id = 0;
				switch (ptr->GetBufferDescriptor().color_attachments[i].get()->GetResourceType())
				{
				case RenderResourceType::RenderTexture2DResource:
					render_id = static_cast<OpenGLRenderTexture2DResource*>(ptr->GetBufferDescriptor().color_attachments[i].get())->GetRenderId();
					break;
				case RenderResourceType::RenderTexture2DArrayResource:
					render_id = static_cast<OpenGLRenderTexture2DArrayResource*>(ptr->GetBufferDescriptor().color_attachments[i].get())->GetRenderId();
					break;
				case RenderResourceType::RenderTexture2DCubemapResource:
					render_id = static_cast<OpenGLRenderTexture2DCubemapResource*>(ptr->GetBufferDescriptor().color_attachments[i].get())->GetRenderId();
					break;
				default:
					throw std::runtime_error("Invalid Framebuffer Texture Type");
				}
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, render_id,0);
				attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
			}
			if(attachments.empty()) {
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
			else {
				glDrawBuffers(attachments.size(), attachments.data());
			}

			unsigned int render_id = 0;
			switch (ptr->GetBufferDescriptor().depth_stencil_attachment.get()->GetResourceType())
			{
			case RenderResourceType::RenderTexture2DResource:
				render_id = static_cast<OpenGLRenderTexture2DResource*>(ptr->GetBufferDescriptor().depth_stencil_attachment.get())->GetRenderId();
				break;
			case RenderResourceType::RenderTexture2DArrayResource:
				render_id = static_cast<OpenGLRenderTexture2DArrayResource*>(ptr->GetBufferDescriptor().depth_stencil_attachment.get())->GetRenderId();
				break;
			case RenderResourceType::RenderTexture2DCubemapResource:
				render_id = static_cast<OpenGLRenderTexture2DCubemapResource*>(ptr->GetBufferDescriptor().depth_stencil_attachment.get())->GetRenderId();
				break;
			default:
				throw std::runtime_error("Invalid Framebuffer Texture Type");
			}
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, render_id,0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				throw std::runtime_error("FrameBuffer Initialization failed");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			static_cast<OpenGLRenderFrameBufferResource*>(ptr.get())->SetRenderId(buffer);
			ptr->SetRenderState(RenderState::COMMON);
		}
		else {
			throw std::runtime_error("Resource has already been Initialied");
		}
		}));
	return ptr;
}

void OpenGLRenderResourceManager::UploadDataToTexture2DArray(RenderCommandList* list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
	size_t size = OpenGLUnitConverter::TextureFormatToTexelSize(resource->GetBufferDescriptor().format) * width * height;
	char* allocated = new char[size];
	memcpy(allocated, data, size);
	static_cast<OpenGLRenderCommandList*>(list)->UpdateTexture2DArrayResource(resource, layer, level, allocated, width, height, offset_x, offset_y);
}

std::shared_ptr<RenderTexture2DCubemapResource> OpenGLRenderResourceManager::CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc)
{
	if (!buffer_desc.sampler) {
		throw std::runtime_error("Sampler wasn't supplied to the texture");
	}
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderTexture2DCubemapResource>(&ResourcePool);
	std::unique_lock<std::mutex> lock(ResourceMutex);
	OpenGLRenderTexture2DCubemapResource* resource = std::allocator_traits<decltype(allocator)>::allocate(allocator, 1);
	lock.unlock();
	std::allocator_traits<decltype(allocator)>::construct(allocator, resource, buffer_desc, RenderState::UNINITIALIZED);
	auto ptr = std::shared_ptr<RenderTexture2DCubemapResource>(static_cast<RenderTexture2DCubemapResource*>(resource), [this](RenderTexture2DCubemapResource* ptr) {
		ReturnTexture2DCubemapResource(ptr);
		});

	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([ptr] {
		if (ptr->GetRenderState() == RenderState::UNINITIALIZED) {
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
			//TODO: The usage choice needs to be reworked

			for (int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, OpenGLUnitConverter::TextureFormatToGLInternalformat(ptr->GetBufferDescriptor().format), ptr->GetBufferDescriptor().res,
					ptr->GetBufferDescriptor().res, 0, OpenGLUnitConverter::TextureFormatToGLFormat(ptr->GetBufferDescriptor().format),
					OpenGLUnitConverter::TextureFormatToGLDataType(ptr->GetBufferDescriptor().format), NULL);
			}

			const TextureSamplerDescritor& sampler = ptr->GetBufferDescriptor().sampler->GetDescriptor();

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_U));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_V));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, OpenGLUnitConverter::TextureAddressModeToGLWrappingMode(sampler.AddressMode_W));
			glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(sampler.border_color));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, OpenGLUnitConverter::TextureFilterToGLMinFilter(sampler.filter));
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, OpenGLUnitConverter::TextureFilterToGLMagFilter(sampler.filter));


			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			static_cast<OpenGLRenderTexture2DCubemapResource*>(ptr.get())->SetRenderId(texture);
			ptr->SetRenderState(RenderState::COMMON);
		}
		else {
			throw std::runtime_error("Resource has already been Initialied");
		}
		}));
	return ptr;
}

void OpenGLRenderResourceManager::UploadDataToTexture2DCubemap(RenderCommandList* list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y, int level)
{
	size_t size = OpenGLUnitConverter::TextureFormatToTexelSize(resource->GetBufferDescriptor().format) * width * height;
	char* allocated = new char[size];
	memcpy(allocated, data, size);
	static_cast<OpenGLRenderCommandList*>(list)->UpdateTexture2DCubemapResource(resource, face, level, allocated, width, height, offset_x, offset_y);
}

//References to Resources never get destroyed !!!!!!!!!!!!!!!!!!!!!!!!!!!!! -> partially fixed (recommend revision)
void OpenGLRenderResourceManager::CreateConstantBufferDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource)
{
	OpenGLRenderDescriptorAllocation* gl_table = static_cast<OpenGLRenderDescriptorAllocation*>(table.get());
	if (index > gl_table->num_of_descriptors) {
		throw std::runtime_error("Descriptor out of range");
	}
	OpenGLResourceDescriptor* desc = &gl_table->descriptor_pointer[index];
	desc->m_resource = resource;
	desc->type = RootParameterType::CONSTANT_BUFFER;

}

//References to Resources never get destoryed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void OpenGLRenderResourceManager::CreateTexture2DDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource)
{
	OpenGLRenderDescriptorAllocation* gl_table = static_cast<OpenGLRenderDescriptorAllocation*>(table.get());
	if (index > gl_table->num_of_descriptors) {
		throw std::runtime_error("Descriptor out of range");
	}
	OpenGLResourceDescriptor* desc = &gl_table->descriptor_pointer[index];
	desc->m_resource = resource;
	desc->type = RootParameterType::TEXTURE_2D;
}

Future<read_pixel_data> OpenGLRenderResourceManager::GetPixelValue(std::shared_ptr<RenderFrameBufferResource> framebuffer, int color_attachment_index, float x, float y)
{
	if (framebuffer->GetBufferDescriptor().color_attachments.size() - 1 < color_attachment_index) throw std::runtime_error("Color attachment on a framebuffer doesn't exist");
	if (framebuffer->GetBufferDescriptor().color_attachments[color_attachment_index]->GetResourceType() != RenderResourceType::RenderTexture2DResource)
		throw std::runtime_error("Unsupported buffer attachment type")
		;
	std::shared_ptr<Promise<read_pixel_data>> data = std::make_shared<Promise<read_pixel_data>>();
	Future<read_pixel_data> future = data->GetFuture();
	std::shared_ptr<RenderTexture2DResource> texture = std::dynamic_pointer_cast<RenderTexture2DResource>(framebuffer->GetBufferDescriptor().color_attachments[color_attachment_index]);
	int x_int = x * texture->GetBufferDescriptor().width;
	int y_int = (1-y) * texture->GetBufferDescriptor().height;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([data, x_int, y_int, texture, color_attachment_index, framebuffer]() {
		int buffer = 0;
		int this_buffer = std::dynamic_pointer_cast<OpenGLRenderFrameBufferResource>(framebuffer)->GetRenderId();
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &buffer);
		if (this_buffer != buffer) {
			glBindFramebuffer(GL_FRAMEBUFFER, this_buffer);
		}
		switch (texture->GetBufferDescriptor().format)
		{
		case TextureFormat::RGBA_32FLOAT:
		{
			glm::vec4 value; //This is just fucked, what am i even doing, i just realized this is a suicide attempt.
			glReadBuffer(GL_COLOR_ATTACHMENT0 + color_attachment_index);
			glReadPixels(x_int, y_int, 1, 1, OpenGLUnitConverter::TextureFormatToGLFormat(texture->GetBufferDescriptor().format), OpenGLUnitConverter::TextureFormatToGLDataType(texture->GetBufferDescriptor().format), glm::value_ptr(value));
			data->SetValue(read_pixel_data(value));
			break;
		}
		case TextureFormat::RGB_32FLOAT:
		{
			glm::vec3 value;
			glReadBuffer(GL_COLOR_ATTACHMENT0 + color_attachment_index);
			glReadPixels(x_int, y_int, 1, 1, OpenGLUnitConverter::TextureFormatToGLFormat(texture->GetBufferDescriptor().format), OpenGLUnitConverter::TextureFormatToGLDataType(texture->GetBufferDescriptor().format), glm::value_ptr(value));
			data->SetValue(read_pixel_data(value));
			break;
		}
		case TextureFormat::R_UNSIGNED_INT:
		{
			unsigned int value;
			glReadBuffer(GL_COLOR_ATTACHMENT0 + color_attachment_index);
			glReadPixels(x_int, y_int, 1, 1, OpenGLUnitConverter::TextureFormatToGLFormat(texture->GetBufferDescriptor().format), OpenGLUnitConverter::TextureFormatToGLDataType(texture->GetBufferDescriptor().format), &value);
			data->SetValue(read_pixel_data(value));
			break;
		}
		default:
			throw std::runtime_error("GetPixelValue currently doesn't support this data type");
		}
		if (this_buffer != buffer) {
			glBindFramebuffer(GL_FRAMEBUFFER, buffer);
		}
		}));

	return future;
}

void OpenGLRenderResourceManager::CreateTexture2DArrayDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource)
{
	OpenGLRenderDescriptorAllocation* gl_table = static_cast<OpenGLRenderDescriptorAllocation*>(table.get());
	if (index > gl_table->num_of_descriptors) {
		throw std::runtime_error("Descriptor out of range");
	}
	OpenGLResourceDescriptor* desc = &gl_table->descriptor_pointer[index];
	desc->m_resource = resource;
	desc->type = RootParameterType::TEXTURE_2D_ARRAY;
}

void OpenGLRenderResourceManager::CreateTexture2DCubemapDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource)
{
	OpenGLRenderDescriptorAllocation* gl_table = static_cast<OpenGLRenderDescriptorAllocation*>(table.get());
	if (index > gl_table->num_of_descriptors) {
		throw std::runtime_error("Descriptor out of range");
	}
	OpenGLResourceDescriptor* desc = &gl_table->descriptor_pointer[index];
	desc->m_resource = resource;
	desc->type = RootParameterType::TEXTURE_2D_CUBEMAP;
}

void OpenGLRenderResourceManager::CopyFrameBufferDepthAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer)
{
	static_cast<OpenGLRenderCommandList*>(list)->CopyFrameBufferDepthAttachment(source_frame_buffer, destination_frame_buffer);
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

void OpenGLRenderResourceManager::ReturnTexture2DArrayResource(RenderTexture2DArrayResource* resource)
{
	OpenGLRenderTexture2DArrayResource* ptr = static_cast<OpenGLRenderTexture2DArrayResource*>(resource);
	unsigned int texture = ptr->render_id;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([texture] {
		glDeleteTextures(1, &texture);
		}));
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderTexture2DArrayResource>(&ResourcePool);
	std::allocator_traits<decltype(allocator)>::destroy(allocator, ptr);
	std::lock_guard<std::mutex> lock(ResourceMutex);
	std::allocator_traits<decltype(allocator)>::deallocate(allocator, ptr, 1);
}

void OpenGLRenderResourceManager::ReturnFrameBufferResource(RenderFrameBufferResource* resource)
{
	OpenGLRenderFrameBufferResource* ptr = static_cast<OpenGLRenderFrameBufferResource*>(resource);
	unsigned int frame_buffer = ptr->render_id;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([frame_buffer] {
		glDeleteFramebuffers(1, &frame_buffer);
		}));
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderFrameBufferResource>(&ResourcePool);
	std::allocator_traits<decltype(allocator)>::destroy(allocator, ptr);
	std::lock_guard<std::mutex> lock(ResourceMutex);
	std::allocator_traits<decltype(allocator)>::deallocate(allocator, ptr, 1);
}

void OpenGLRenderResourceManager::ReturnTexture2DCubemapResource(RenderTexture2DCubemapResource* resource)
{
	OpenGLRenderTexture2DCubemapResource* ptr = static_cast<OpenGLRenderTexture2DCubemapResource*>(resource);
	unsigned int texture = ptr->render_id;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([texture] {
		glDeleteTextures(1, &texture);
		}));
	auto allocator = std::pmr::polymorphic_allocator<OpenGLRenderTexture2DCubemapResource>(&ResourcePool);
	std::allocator_traits<decltype(allocator)>::destroy(allocator, ptr);
	std::lock_guard<std::mutex> lock(ResourceMutex);
	std::allocator_traits<decltype(allocator)>::deallocate(allocator, ptr, 1);
}
