#include "RenderDescriptorHeap.h"
#include <FrameManager.h>
#include <stdexcept>

RenderDescriptorHeap::RenderDescriptorHeap(size_t descriptor_heap_size) : descriptor_block(RenderDescriptorHeapBlock::CreateHeapBlock(descriptor_heap_size))
{

}

RenderDescriptorHeap::~RenderDescriptorHeap()
{
	FlushDescriptorDeallocations(-1);
	delete descriptor_block;
}

RenderDescriptorAllocationHandle RenderDescriptorHeap::Allocate(size_t num_of_descriptors)
{
	RenderDescriptorAllocation* allocation = descriptor_block->Allocate(num_of_descriptors);
	if (allocation) {
		return std::shared_ptr<RenderDescriptorAllocation>(allocation);
	}
	else {
		throw std::runtime_error("Allocation failed");
	}
}
