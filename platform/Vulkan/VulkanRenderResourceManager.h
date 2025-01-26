#pragma once
#include <Renderer/RenderResourceManager.h>
#include <platform/Vulkan/VulkanRenderCommandList.h>
#include "VulkanRenderResource.h"
#include <Utilities/MemoryManagement/include/MultiPool.h>
#include <mutex>
#include <memory>
#include <queue>
#include <map>
#include <mutex>

class VulkanRenderResourceManager : public RenderResourceManager {
public:

	virtual void Update() override;

	friend RenderResourceManager;
	friend class VulkanRenderContext;

	virtual std::shared_ptr<RenderBufferResource> CreateBuffer(const RenderBufferDescriptor& buffer_desc) override;
	virtual void UploadDataToBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset) override;
	virtual void ReallocateAndUploadBuffer(RenderCommandList* list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size) override;

	virtual std::shared_ptr<RenderTexture2DResource> CreateTexture(const RenderTexture2DDescriptor& buffer_desc, RenderState default_state = RenderState::TEXTURE_SAMPLE) override;
	virtual void UploadDataToTexture2D(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;
	virtual void GenerateMIPs(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource) override;
	virtual void UploadToTexture2DFromFile(RenderCommandList* list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level = 0) override;
	virtual std::shared_ptr<RenderTexture2DResource> CreateTextureFromFile(RenderCommandList* list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler) override;

	virtual std::shared_ptr<RenderTexture2DArrayResource> CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc, RenderState default_state = RenderState::TEXTURE_SAMPLE) override;
	virtual void UploadDataToTexture2DArray(RenderCommandList* list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;

	virtual std::shared_ptr<RenderTexture2DCubemapResource> CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc, RenderState default_state = RenderState::TEXTURE_SAMPLE) override;
	virtual void UploadDataToTexture2DCubemap(RenderCommandList* list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;

	virtual std::shared_ptr<RenderFrameBufferResource> CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc) override;

	virtual void CreateConstantBufferDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource) override;
	virtual void CreateTexture2DDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource) override;
	virtual Future<read_pixel_data> GetPixelValue(std::shared_ptr<RenderFrameBufferResource> framebuffer, int color_attachment_index, float x, float y) override;
	virtual void CreateTexture2DArrayDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource) override;
	virtual void CreateTexture2DCubemapDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource) override;

	virtual void CopyFrameBufferDepthAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer) override;
	virtual void SetFrameBufferColorAttachment(RenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> framebuffer, std::shared_ptr<RenderResource> new_attachment, int index = 0, int level = 0) override;

	void ReturnResource(VulkanRenderResource* resource);

	void BufferBarrier(RenderCommandList* list, std::shared_ptr<RenderBufferResource> buffer, bool make_memory_available = true,
		PipelineStage write_scope = PipelineStage::PIPELINE_BOTTOM, PipelineStage read_scope = PipelineStage::PIPELINE_TOP);

	void TransitionImage(RenderCommandList* list, std::shared_ptr<VulkanRenderTextureResource> image, VkImageSubresourceRange range, RenderState source_state, RenderState target_state, PipelineStage source_scope = PipelineStage::PIPELINE_BOTTOM, PipelineStage target_scope = PipelineStage::PIPELINE_TOP);

	std::shared_ptr<RenderBufferResource> GetStagingBuffer(size_t size);

	VulkanDependencyHandler* GetDependencyHandler();
	void ReturnDependencyHandler(VulkanDependencyHandler* handler);

private:
	VulkanRenderResourceManager();
	~VulkanRenderResourceManager();

	void CreateBuffer_internal(VulkanRenderBufferResource* buffer, const RenderBufferDescriptor& buffer_desc);
	void FlushDeletions();
	void ReturnStagingBufferResource(VulkanRenderBufferResource* resource);
	void ClearStagingBuffers();

	//Creates a texture object from a VkImage which is not managed by the resource manager (used mainly for swapchain textures)
	VulkanRenderTexture2DResource* CreateNonManagedTexture(VkImage image, RenderTexture2DDescriptor desc, RenderState default_state = RenderState::TEXTURE_SAMPLE);



private:
	struct deletion_item {
		VulkanRenderResource* resource = nullptr;
		bool staging = false;
	};

	std::mutex deletion_queue_mutex;
	std::queue<deletion_item> deletion_queue;
	std::mutex staging_buffer_map_mutex;
	std::map<size_t, RenderBufferResource*> staging_buffer_map;
	std::mutex dependency_handler_mutex;
	std::vector<VulkanDependencyHandler*> dependency_handlers;
};