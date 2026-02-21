#pragma once
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <Renderer/RenderContext.h>
#include <Core/FrameMultiBufferResource.h>
#define VMA_VULKAN_VERSION 1003000 
#include <vk_mem_alloc.h>

#define DEFINE_VK_INSTANCE(x) auto x = static_cast<VulkanRenderContext*>(RenderContext::Get());

extern PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet_KHR;

class VulkanRenderContext : public RenderContext {
public:

	virtual void Init() override;
	virtual void PreInit() override;
	virtual void StartShutdown()override;
	virtual ~VulkanRenderContext() override;
	virtual bool IsVulkanContext() override { return true; };
	void InstanceInit();

	VulkanRenderContext(const VulkanRenderContext& ref) = delete;
	VulkanRenderContext(VulkanRenderContext&& ref) = delete;
	VulkanRenderContext& operator=(const VulkanRenderContext& ref) = delete;
	VulkanRenderContext& operator=(VulkanRenderContext&& ref) = delete;

	virtual void Present() override;


	struct BindlessLimits {
		uint32_t max_bindless_textures = 0;
		uint32_t max_bindless_uniform_buffers = 0;
		uint32_t max_bindless_storage_buffers = 0;
	};

	uint64_t GetCurrentGpuTimelineValue();
	uint64_t GetCurrentCpuTimelineValue();
	VmaAllocator& GetVmaAllocator() { return allocator;  }
	VkInstance GetVkInstance() const { return vk_instance; }
	vkb::Instance GetVkbInstance() const { return vkb_instance; }
	VkDevice GetVkDevice() const { return vk_device; }
	vkb::Device GetVkbDevice() const { return vkb_device; }
	void RequestExtension(const std::string& extension);
	void RequestExtensions(const char** extensions, int count);
	std::vector<const char*> GetExtensions();
	const VkPhysicalDeviceDescriptorIndexingProperties& GetIndexingProperties() const { return indexing_properties; }
	const BindlessLimits& GetBindlessLimits() const { return bindless_limits; }

protected:

	virtual void Destroy() override;

private:
	VulkanRenderContext();
	friend RenderContext;
	friend class VulkanRenderCommandList;
	std::vector<std::string> requested_extensions;
	VkInstance vk_instance;
	vkb::Instance vkb_instance;
	VkDevice vk_device;
	vkb::Device vkb_device;
	VmaAllocator allocator;
	VkPhysicalDeviceDescriptorIndexingProperties indexing_properties;
	BindlessLimits bindless_limits;
};