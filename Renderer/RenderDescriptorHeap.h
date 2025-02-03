#pragma once 
#include <Renderer/RenderDescriptorHeapBlock.h>
#include <memory>
#include <vector>
#include <mutex>

using RenderDescriptorAllocationHandle = std::shared_ptr<RenderDescriptorAllocation>;
using RenderDescriptorTable = std::shared_ptr<RenderDescriptorAllocation>;

class RenderDescriptorHeap {
public:
	RenderDescriptorHeap() = default;
	RenderDescriptorHeap(const RenderDescriptorHeap& ref) = delete;
	RenderDescriptorHeap& operator=(const RenderDescriptorHeap& ref) = delete;
	virtual ~RenderDescriptorHeap() {}

	virtual RenderDescriptorAllocationHandle Allocate() = 0;
	virtual void FlushDescriptorDeallocations(uint32_t frame_number) { };
}; 