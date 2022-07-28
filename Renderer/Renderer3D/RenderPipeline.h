#pragma once 
#include "RenderPass.h"
#include <algorithm>
#include <unordered_map>

struct RenderPassData {
	std::shared_ptr<RenderPass> render_pass;
	RenderPassResourceDefinnition def;
};

class RenderPipelineResourceManager {
public:

	template<typename T>
	const T& GetResource(const std::string& name) const {
		auto& desc_list = current_pass->def.descriptors;
		decltype(current_pass->def.descriptors)::iterator fnd = std::find_if(desc_list.begin(), desc_list.end(), [&name](const RenderPassResourceDescriptor& descriptor) {
			return descriptor.resource_name == name;
			});

		if (fnd != desc_list.end()) {
			if (fnd->type_id != RuntimeTag<T>::GetId()) {
				throw std::runtime_error("Type mismatch when getting resource " + fnd->resource_name);
			}
			if (fnd->desc_access == RenderPassResourceDescriptor_Access::WRITE) {
				throw std::runtime_error("Resource " + fnd->resource_name + " is write only in this context");
			}

			return *static_cast<T*>(pipeline->resources[name].ptr);

		}
		else {
			throw std::runtime_error("Resource " + name + " isn't valid in this context");
		}


	}

	template<typename T>
	void SetResource(const std::string& name, T* resource) {
		auto& desc_list = current_pass->def.descriptors;
		decltype(current_pass->def.descriptors)::iterator fnd = std::find_if(desc_list.begin(), desc_list.end(), [&name](const RenderPassResourceDescriptor& descriptor) {
			return descriptor.resource_name == name;
			});

		if (fnd != desc_list.end()) {
			if (fnd->type_id != RuntimeTag<T>::GetId()) {
				throw std::runtime_error("Type mismatch when getting resource " + fnd->resource_name);
			}
			if (fnd->desc_access == RenderPassResourceDescriptor_Access::READ) {
				throw std::runtime_error("Resource " + fnd->resource_name + " is read only in this context");
			}

			pipeline->resources[name].ptr = resource;

		}
		else {
			throw std::runtime_error("Resource " + fnd->resource_name + " isn't valid in this context");
		}
	}

private:
	RenderPipeline* pipeline = nullptr;
	RenderPassData* current_pass = nullptr;
	friend class RenderPipeline;
	RenderPipelineResourceManager(RenderPipeline* pipeline, RenderPassData* current_pass);
};


class RenderPipeline {
public:

	void Render();

private:
	friend class RenderPipelineResourceManager;
	friend class RenderPassBuilder;
	struct RenderPipelineResourceData_internal {
		void* ptr;
		RenderPassResourceDescriptor desc;
	};
	RenderPipeline(std::vector<RenderPassData>&& render_passes, std::unordered_map<std::string, RenderPipelineResourceData_internal>&& resources);
	std::unordered_map<std::string, RenderPipelineResourceData_internal> resources;
	std::vector<RenderPassData> passes;

};