#include "OpenGLRenderDescriptorHeapBlock.h"
#include <FrameManager.h>

RenderDescriptorAllocation* OpenGLRenderDescriptorHeapBlock::Allocate(size_t num_of_descriptors)
{
	std::lock_guard<std::mutex> lock(map_mutex);
	if (num_of_descriptors > free_descriptors) {
		return nullptr;
	}

	auto size_iter = m_SizeTable.lower_bound(num_of_descriptors);

	if (size_iter == m_SizeTable.end()) {
		return nullptr;
	}

	size_t block_size = size_iter->first;
	auto offset_iter = size_iter->second;
	size_t offset = offset_iter->first;

	size_t new_block_size = num_of_descriptors;
	size_t new_block_offset = offset;

	size_t old_block_size = block_size - new_block_size;
	size_t old_block_offset = offset + new_block_size;
	m_SizeTable.erase(size_iter);
	m_OffsetTable.erase(offset_iter);

	if (old_block_size) {
		auto old_block_iter = m_OffsetTable.insert(std::make_pair(old_block_offset, InternalAllocation(m_SizeTable.end())));
		auto size_block_iter_old = m_SizeTable.insert(std::make_pair(old_block_size, old_block_iter.first));
		old_block_iter.first->second.size_map_entry = size_block_iter_old.first;
	}

	free_descriptors -= num_of_descriptors;

	return new OpenGLRenderDescriptorAllocation(m_DescriptorPool.data() + new_block_offset, new_block_size, this);

}

void OpenGLRenderDescriptorHeapBlock::FlushDescriptorDeallocations(uint32_t frame_number)
{
	std::lock_guard<std::mutex> lock(stale_mutex);
	while (!stale_descriptors.empty() && stale_descriptors.front().frame_number <= frame_number) {
		const auto& current_desc = stale_descriptors.front();
		stale_descriptors.pop_front();
		FreeInternal(current_desc);
	}
}

void OpenGLRenderDescriptorHeapBlock::FreeAllocation(OpenGLRenderDescriptorAllocation* allocation, uint32_t frame_number)
{
	InternalFreeStructure free;
	free.descriptor_pointer = allocation->descriptor_pointer;
	free.num_of_descriptors = allocation->num_of_descriptors;
	free.frame_number = frame_number;
	std::lock_guard<std::mutex> lock(stale_mutex);
	stale_descriptors.push_back(free);
}

void OpenGLRenderDescriptorHeapBlock::FreeInternal(const InternalFreeStructure& free_block)
{

	for (int i = 0; i < free_block.num_of_descriptors; i++) {
		free_block.descriptor_pointer[i].m_resource.reset();
		free_block.descriptor_pointer[i].type = RootParameterType::UNDEFINED;
	}

	size_t new_offset = size_t(free_block.descriptor_pointer - m_DescriptorPool.data());
	size_t new_size = free_block.num_of_descriptors;

	std::lock_guard<std::mutex> lock(map_mutex);

	auto upper_iter = m_OffsetTable.upper_bound(new_offset);
	
	auto lower_iter = upper_iter;

	if (lower_iter == m_OffsetTable.begin()) {
		lower_iter = m_OffsetTable.end();
	}
	else {
		lower_iter--;
	}

	if(lower_iter != m_OffsetTable.end() && (lower_iter->first + lower_iter->second.size_map_entry->first) == new_offset) {
		new_offset = lower_iter->first;
		new_size += lower_iter->second.size_map_entry->first;
		m_SizeTable.erase(lower_iter->second.size_map_entry);
		m_OffsetTable.erase(lower_iter);
	}

	if (upper_iter != m_OffsetTable.end() && (upper_iter->first) == (new_offset + new_size)) {
		new_size += upper_iter->second.size_map_entry->first;
		m_SizeTable.erase(upper_iter->second.size_map_entry);
		m_OffsetTable.erase(upper_iter);
	}

	auto offset_iter = m_OffsetTable.insert(std::make_pair(new_offset, InternalAllocation(m_SizeTable.end())));
	auto size_iter = m_SizeTable.insert(std::make_pair(new_size, offset_iter.first));
	offset_iter.first->second.size_map_entry = size_iter.first;

	free_descriptors += free_block.num_of_descriptors;
}

OpenGLRenderDescriptorHeapBlock::OpenGLRenderDescriptorHeapBlock(size_t size) 
	: m_DescriptorPool(), m_OffsetTable(), m_SizeTable(), max_descriptors(size), free_descriptors(size), stale_descriptors(), stale_mutex(), map_mutex()
{
	m_DescriptorPool.reserve(size);
	for (int i = 0; i < size; i++) {
		m_DescriptorPool.emplace_back();
	}
	auto offset_iter = m_OffsetTable.insert(std::make_pair(0, InternalAllocation(m_SizeTable.end())));
	auto size_iter = m_SizeTable.insert(std::make_pair(size, offset_iter.first));
	offset_iter.first->second.size_map_entry = size_iter.first;
}

OpenGLRenderDescriptorAllocation::~OpenGLRenderDescriptorAllocation()
{
	owning_block->FreeAllocation(this, FrameManager::Get()->GetCurrentFrameNumber());
}
