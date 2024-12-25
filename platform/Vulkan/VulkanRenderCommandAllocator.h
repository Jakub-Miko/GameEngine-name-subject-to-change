#pragma once
#include <Renderer/RenderCommandAllocator.h>
#include <vulkan/vulkan.h>

class VulkanRenderCommandAllocator  : public RenderCommandAllocator {
public:
	virtual void* Get() override;
	virtual void clear() override;

	VulkanRenderCommandAllocator(size_t starting_size);

	virtual ~VulkanRenderCommandAllocator();

private:
	VkCommandPool pool;
};
