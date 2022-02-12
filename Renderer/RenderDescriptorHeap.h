#pragma once 
#include <Renderer/RenderDescriptorHeapBlock.h>
#include <memory>
#include <vector>
#include <mutex>

using RenderDescriptorAllocationHandle = std::shared_ptr<RenderDescriptorAllocation>;
using RenderDescriptorTable = std::shared_ptr<RenderDescriptorAllocation>;

class RenderDescriptorHeap {
public:

	RenderDescriptorHeap(size_t descriptor_heap_size);
	RenderDescriptorHeap(const RenderDescriptorHeap& ref) = delete;
	RenderDescriptorHeap& operator=(const RenderDescriptorHeap& ref) = delete;
	~RenderDescriptorHeap();

	RenderDescriptorAllocationHandle Allocate(size_t num_of_descriptors);
	void FlushDescriptorDeallocations(uint32_t frame_number);

private:

	RenderDescriptorHeapBlock* descriptor_block;

}; 