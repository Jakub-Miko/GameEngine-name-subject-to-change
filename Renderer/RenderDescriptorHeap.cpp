#include "RenderDescriptorHeap.h"
#include <FrameManager.h>
#include <stdexcept>

RenderDescriptorHeap::RenderDescriptorHeap(size_t descriptor_heap_size) : descriptor_block(RenderDescriptorHeapBlock::CreateHeapBlock(descriptor_heap_size))
{

}

RenderDescriptorHeap::~RenderDescriptorHeap()
{
	FlushDescriptorDeallocations(FrameManager::Get()->GetCurrentFrameNumber());
	delete descriptor_block;
}

RenderDescriptorAllocationHandle RenderDescriptorHeap::Allocate(size_t num_of_descriptors)
{
	RenderDescriptorAllocation* allocation = descriptor_block->Allocate(num_of_descriptors);
	if (allocation) {
		return std::unique_ptr<RenderDescriptorAllocation>(allocation);
	}
	else {
		throw std::runtime_error("Allocation failed");
	}
}

void RenderDescriptorHeap::FlushDescriptorDeallocations(uint32_t frame_number)
{
	descriptor_block->FlushDescriptorDeallocations(frame_number);
}
