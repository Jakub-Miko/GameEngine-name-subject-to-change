#pragma once 
#include <memory>
#include "RenderResource.h"
#include <Renderer/RenderDescriptorHeap.h>
#include <Renderer/Renderer.h>
#include <variant>
#include <Promise.h>


using read_pixel_data = typename std::template variant<glm::vec4, glm::vec3, glm::vec2, glm::uvec4, glm::uvec2, float, unsigned int, char>;


class RenderResourceManager {
public:
	
	static void Initialize();
	static RenderResourceManager* Get();
	static void Shutdown();

	virtual std::shared_ptr<RenderBufferResource> CreateBuffer(const RenderBufferDescriptor& buffer_desc) = 0;
	virtual void UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset) = 0;
	virtual void ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size) = 0;

	virtual std::shared_ptr<RenderTexture2DResource> CreateTexture(const RenderTexture2DDescriptor& buffer_desc) = 0;
	virtual void UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) = 0;
	virtual void GenerateMIPs(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource) = 0;
	virtual void UploadToTexture2DFromFile(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level = 0) = 0;
	virtual std::shared_ptr<RenderTexture2DResource> CreateTextureFromFile(RenderCommandList* list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler) = 0;

	virtual std::shared_ptr<RenderTexture2DArrayResource> CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc) = 0;
	virtual void UploadDataToTexture2DArray(RenderCommandList* list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) = 0;

	virtual std::shared_ptr<RenderTexture2DCubemapResource> CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc) = 0;
	virtual void UploadDataToTexture2DCubemap(RenderCommandList* list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) = 0;

	virtual std::shared_ptr<RenderFrameBufferResource> CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc) = 0;

	virtual void CreateConstantBufferDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource) = 0;
	virtual void CreateTexture2DDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource) = 0;
	virtual Future<read_pixel_data> GetPixelValue(std::shared_ptr<RenderFrameBufferResource> framebuffer, int color_attachment_index, float x, float y) = 0;
	virtual void CreateTexture2DArrayDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource) = 0;
	virtual void CreateTexture2DCubemapDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource) = 0;

	//This may reset the framebuffer bindings
	virtual void CopyFrameBufferDepthAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer) = 0;

	virtual ~RenderResourceManager() {};

private:
	static RenderResourceManager* instance;
};