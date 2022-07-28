#pragma once
#include <Renderer/RenderResource.h>
#include <string>
#include <Core/RuntimeTag.h>
#include <vector>

class RenderPipelineResourceManager;

enum class RenderPassResourceDescriptor_Access {
	READ = 0, WRITE = 1
};

struct RenderPassResourceDescriptor {
	std::string resource_name;
	RuntimeTagIdType type_id;
	RenderPassResourceDescriptor_Access desc_access;
};

class RenderPassResourceDefinnition {
public:

	template<typename T>
	void AddResource(std::string resource_name, RenderPassResourceDescriptor_Access access) {
		RuntimeTagIdType id = RuntimeTag<T>::GetId();
		if (id == -1) {
			throw std::runtime_error("Resource with name " + resource_name + " doesn't have a runtime identifier");
		}
		RenderPassResourceDescriptor desc;
		desc.type_id = id;
		desc.resource_name = resource_name;
		desc.desc_access = access;
		descriptors.push_back(desc);
	}

private:
	friend class RenderPassBuilder;
	friend class RenderPipeline;
	friend class RenderPipelineResourceManager;
	std::vector<RenderPassResourceDescriptor> descriptors;

};

class RenderPass {
public:

	virtual void Setup(RenderPassResourceDefinnition& setup_builder) = 0;
	virtual void Render(RenderPipelineResourceManager& resource_manager) = 0;

};