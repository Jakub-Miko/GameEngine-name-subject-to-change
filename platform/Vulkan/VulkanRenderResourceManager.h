#pragma once
#include <Renderer/RenderResourceManager.h>
#include <platform/Vulkan/VulkanRenderCommandList.h>
#include "VulkanRenderResource.h"
#include <Utilities/MemoryManagement/include/MultiPool.h>
#include "VulkanRenderCommandList.h"
#include "VulkanDeferredDestruction.h"
#include <mutex>
#include <memory>
#include <queue>
#include <map>
#include <mutex>
#include <variant>

class VulkanRenderResourceManager : public RenderResourceManager {
public:

	virtual void Update() override;

	friend RenderResourceManager;
	friend class VulkanRenderContext;

	virtual std::shared_ptr<RenderBufferResource> CreateBuffer(const RenderBufferDescriptor& buffer_desc, RenderBufferCreationFlags flags = RenderBufferCreationFlags::NONE) override;
	virtual void UploadDataToBuffer(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset) override;
	virtual void ReallocateAndUploadBuffer(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderBufferResource> resource, void* data, size_t size) override;
	virtual void CopyBufferData(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderBufferResource> source, std::shared_ptr<RenderBufferResource> destination 
		, size_t source_offset, size_t source_size, size_t destination_offset) override;

	virtual std::shared_ptr<RenderTexture2DResource> CreateTexture(const RenderTexture2DDescriptor& buffer_desc, RenderState default_state = RenderState::TEXTURE_SAMPLE) override;
	virtual void UploadDataToTexture2D(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DResource> resource, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;
	virtual void GenerateMIPs(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DResource> resource) override;
	virtual void UploadToTexture2DFromFile(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DResource> resource, const std::string& filepath, int level = 0) override;
	virtual std::shared_ptr<RenderTexture2DResource> CreateTextureFromFile(std::shared_ptr<RenderCommandList>  list, const std::string& filepath, std::shared_ptr<TextureSampler> sampler) override;

	virtual std::shared_ptr<RenderTexture2DArrayResource> CreateTextureArray(const RenderTexture2DArrayDescriptor& buffer_desc, RenderState default_state = RenderState::TEXTURE_SAMPLE) override;
	virtual void UploadDataToTexture2DArray(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;

	virtual std::shared_ptr<RenderTexture2DCubemapResource> CreateTextureCubemap(const RenderTexture2DCubemapDescriptor& buffer_desc, RenderState default_state = RenderState::TEXTURE_SAMPLE) override;
	virtual void UploadDataToTexture2DCubemap(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderTexture2DCubemapResource> resource, CubemapFace face, void* data, size_t width, size_t height,
		size_t offset_x, size_t offset_y, int level = 0) override;

	virtual std::shared_ptr<RenderFrameBufferResource> CreateFrameBuffer(const RenderFrameBufferDescriptor& buffer_desc) override;

	virtual void CreateConstantBufferDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderBufferResource> resource) override;
	virtual void CreateTexture2DDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DResource> resource) override;
	virtual std::shared_ptr<Awaitable<read_pixel_data>> GetPixelValue(std::shared_ptr<RenderFrameBufferResource> framebuffer, int color_attachment_index, float x, float y) override;
	virtual void CreateTexture2DArrayDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DArrayResource> resource) override;
	virtual void CreateTexture2DCubemapDescriptor(const RenderDescriptorTable& table, int index, std::shared_ptr<RenderTexture2DCubemapResource> resource) override;

	virtual void CopyFrameBufferDepthAttachment(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer) override;
	virtual void SetFrameBufferColorAttachment(std::shared_ptr<RenderCommandList>  list, std::shared_ptr<RenderFrameBufferResource> framebuffer, std::shared_ptr<RenderResource> new_attachment, int index = 0, int level = 0) override;

	virtual int GetCubemapFaceIndex(RenderCubemapFace face) override;

	void ReturnResource(VulkanRenderResource* resource);

	void BufferBarrier(RenderCommandList* list, std::shared_ptr<RenderBufferResource> buffer,
		PipelineStage write_scope = PipelineStage::ALL_STAGES, PipelineStage read_scope = PipelineStage::ALL_STAGES,
		VulkanCommandListDependencyType src_access = VulkanCommandListDependencyType::READ, VulkanCommandListDependencyType dst_access = VulkanCommandListDependencyType::WRITE);

	void TransitionImage(RenderCommandList* list, VulkanRenderTextureResource* image, VkImageSubresourceRange range, RenderState source_state, RenderState target_state,
		PipelineStage source_scope = PipelineStage::ALL_STAGES, PipelineStage target_scope = PipelineStage::ALL_STAGES, 
		VulkanCommandListDependencyType src_access = VulkanCommandListDependencyType::READ, VulkanCommandListDependencyType dst_access = VulkanCommandListDependencyType::WRITE);

	std::shared_ptr<RenderBufferResource> GetStagingBuffer(size_t size);

	void AddToDeferredDestructionQueue(VulkanDeferredDestruction* resource, uint32_t last_usage_timeline_value);

	void ReturnDescriptorAllocation(RenderDescriptorAllocation* allocation, uint64_t deletion_timeline);
	
	//Creates a texture object from a VkImage which is not managed by the resource manager (used mainly for swapchain textures)
	VulkanRenderTexture2DResource* CreateNonManagedTexture(VkImage image, VkImageView view, RenderTexture2DDescriptor desc, RenderState default_state = RenderState::TEXTURE_SAMPLE, RenderState initial_state = RenderState::UNINITIALIZED);

private:
	VulkanRenderResourceManager();
	~VulkanRenderResourceManager();

	void CreateBuffer_internal(VulkanRenderBufferResource* buffer, const RenderBufferDescriptor& buffer_desc, RenderBufferCreationFlags flags);
	void FlushDeletions(bool force = false);
	void ReturnStagingBufferResource(VulkanRenderBufferResource* resource);
	void ClearStagingBuffers();




private:
	enum class deletion_item_type : char {
		RESOURCE, STAGING_BUFFER, DESCRIPTOR_ALLOCATION, DEFERRED_DESTROY_RESOURCE
	};

	struct deletion_item {
		union {
			VulkanRenderResource* resource;
			RenderDescriptorAllocation* descriptor_allocation;
			VulkanDeferredDestruction* deferred_destroy_resource;
		};
		uint64_t deletion_timeline;
		deletion_item_type type = deletion_item_type::RESOURCE;
	};

	std::mutex deletion_queue_mutex;
	std::queue<deletion_item> deletion_queue;
	std::mutex staging_buffer_map_mutex;
	std::multimap<size_t, RenderBufferResource*> staging_buffer_map;
};