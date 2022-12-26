#pragma once
#include <Renderer/RenderResource.h>
#include <string>
#include <Core/RuntimeTag.h>
#include <vector>
#include <any>

struct DependencyTag {
	RUNTIME_TAG("DependencyTag");
};

class RenderPipelineResourceManager;

enum class RenderPassResourceDescriptor_Access {
	READ = 0, WRITE = 1
};

struct RenderPassResourceDescriptor {
	std::string resource_name;
	RuntimeTagIdType type_id;
	RenderPassResourceDescriptor_Access desc_access;
};

struct PersistentRenderPassResource {
	std::any resource;
	RenderPassResourceDescriptor resource_desc;
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

	template<typename T>
	void AddPersistentResource(const std::string& name, const T& resource) {
		PersistentRenderPassResource persistent_data;
		persistent_data.resource_desc.desc_access = RenderPassResourceDescriptor_Access::READ;
		persistent_data.resource_desc.resource_name = name;
		persistent_data.resource_desc.type_id = RuntimeTag<T>::GetId();
		persistent_data.resource = resource;
		persistent_resources.push_back(persistent_data);
	}

private:
	friend class RenderPassBuilder;
	friend class RenderPipeline;
	friend class RenderPipelineResourceManager;
	std::vector<RenderPassResourceDescriptor> descriptors;
	std::vector< PersistentRenderPassResource> persistent_resources;
};

class RenderPass {
public:
	virtual ~RenderPass() {};
	virtual void Setup(RenderPassResourceDefinnition& setup_builder) = 0;
	virtual void Render(RenderPipelineResourceManager& resource_manager) = 0;

};