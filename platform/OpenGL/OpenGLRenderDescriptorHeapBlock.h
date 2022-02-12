#pragma once
#include <Renderer/RenderDescriptorHeapBlock.h>
#include <map>
#include <vector>
#include <Renderer/RendererDefines.h>
#include <Renderer/RenderResource.h>
#include <deque>
#include <mutex>

struct OpenGLResourceDescriptor : public RenderResourceDescriptor {
	OpenGLResourceDescriptor() : m_resource(nullptr), type(RootParameterType::UNDEFINED) {}
	std::shared_ptr<RenderResource> m_resource;
	RootParameterType type;
};

class OpenGLRenderDescriptorHeapBlock;

class OpenGLRenderDescriptorAllocation : public RenderDescriptorAllocation {
public:
	OpenGLRenderDescriptorAllocation(OpenGLResourceDescriptor* descriptor_pointer, size_t num_of_descriptors, OpenGLRenderDescriptorHeapBlock* owning_block) :
		descriptor_pointer(descriptor_pointer), num_of_descriptors(num_of_descriptors), owning_block(owning_block) {}

	virtual ~OpenGLRenderDescriptorAllocation();
	OpenGLResourceDescriptor* descriptor_pointer = nullptr;
	size_t num_of_descriptors = 0;
	OpenGLRenderDescriptorHeapBlock* owning_block = nullptr;
};


class OpenGLRenderDescriptorHeapBlock : public RenderDescriptorHeapBlock {
public:
	friend RenderDescriptorHeapBlock;
	friend OpenGLRenderDescriptorAllocation;
	virtual ~OpenGLRenderDescriptorHeapBlock() {}

	virtual RenderDescriptorAllocation* Allocate(size_t num_of_descriptors) override;

	virtual void FlushDescriptorDeallocations(uint32_t frame_number) override;

	void FreeAllocation(OpenGLRenderDescriptorAllocation* allocation, uint32_t frame_number);

private:

	struct InternalFreeStructure {
		OpenGLResourceDescriptor* descriptor_pointer;
		size_t num_of_descriptors;
		uint32_t frame_number;
	};

	void FreeInternal(const InternalFreeStructure& free_block);

	OpenGLRenderDescriptorHeapBlock(size_t size);

private:
	struct InternalAllocation;
	
	using OffsetMap = std::map<size_t, InternalAllocation>;
	using SizeMap = std::map<size_t, OffsetMap::iterator>;
	
	struct InternalAllocation {
		InternalAllocation(SizeMap::iterator iter) : size_map_entry(iter) {};
		SizeMap::iterator size_map_entry;
	};

	OffsetMap m_OffsetTable;
	SizeMap m_SizeTable;

	size_t max_descriptors = 0;
	size_t free_descriptors = 0;


	std::deque<InternalFreeStructure> stale_descriptors;

	std::vector<OpenGLResourceDescriptor> m_DescriptorPool;

	std::mutex map_mutex;
	std::mutex stale_mutex;

};