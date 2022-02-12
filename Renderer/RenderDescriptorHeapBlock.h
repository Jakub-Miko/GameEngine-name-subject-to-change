#pragma once
#include <Renderer/RendererDefines.h>
#include <stdint.h>

class RenderDescriptorAllocation {
public:
	virtual ~RenderDescriptorAllocation() {}
};

class RenderDescriptorHeapBlock {
public:

	static RenderDescriptorHeapBlock* CreateHeapBlock(size_t size);

	virtual RenderDescriptorAllocation* Allocate(size_t num_of_descriptors) = 0;
	
	virtual void FlushDescriptorDeallocations(uint32_t frame_number) = 0;

	virtual ~RenderDescriptorHeapBlock() {}

};