#include "RenderPassBuilder.h"
#include <unordered_set>

RenderPassBuilder::RenderPassBuilder() : resource_data(), render_passes()
{
}

void RenderPassBuilder::AddPass(RenderPass* render_pass)
{

	RenderPassBuilder_RenderPass_data render_pass_data;
	render_pass_data.render_pass = std::shared_ptr<RenderPass>(render_pass);
	RenderPassResourceDefinnition def;
	render_pass->Setup(def);
	render_pass_data.resources = def;
	render_passes.push_back(render_pass_data);

	std::unordered_set<std::string> names;

	for (auto resource : def.descriptors) {
		RenderPassBuilder_Resource_data* data;
		auto check = names.find(resource.resource_name);
		if (check == names.end()) {
			names.insert(resource.resource_name);
		}
		else {
			throw std::runtime_error("Duplicate resource name " + resource.resource_name);
		}
		auto fnd = resource_data.find(resource.resource_name);
		if (fnd != resource_data.end()) {
			data = &fnd->second;
			if (data->desc.type_id != resource.type_id) {
				throw std::runtime_error("Type mismatch in resource " + resource.resource_name);
			}
		}
		else {
			auto new_res = resource_data.insert(std::make_pair(resource.resource_name, RenderPassBuilder_Resource_data{ resource }));
			data = &new_res.first->second;
		}
		
		switch (resource.desc_access)
		{
		case RenderPassResourceDescriptor_Access::READ:
			data->consumer_index_list.push_front(render_passes.size() - 1);
			break;
		case RenderPassResourceDescriptor_Access::WRITE:
			if (data->creator_index != -1) {
				throw std::runtime_error("RenderPass resource can currently only have one writer");
			}
			data->creator_index = render_passes.size() - 1;
			break;
		default:
			throw std::runtime_error("Invalid Resource Access");
		}
	}
	std::unordered_set<std::string> persistent_names;

	for (auto persistent_data : def.persistent_resources) {
		auto check = persistent_names.find(persistent_data.resource_desc.resource_name);
		if (check == persistent_names.end()) {
			names.insert(persistent_data.resource_desc.resource_name);
		}
		else {
			throw std::runtime_error("Duplicate persistent resource name " + persistent_data.resource_desc.resource_name);
		}
		auto fnd = persistent_resources.find(persistent_data.resource_desc.resource_name);
		if (fnd != persistent_resources.end()) {
			if (fnd->second.resource_desc.type_id != persistent_data.resource_desc.type_id) {
				throw std::runtime_error("Type mismatch in resource " + persistent_data.resource_desc.resource_name);
			}
		}
		else {
			auto new_res = persistent_resources.insert(std::make_pair(persistent_data.resource_desc.resource_name, persistent_data));
		}
	}

}

RenderPipeline RenderPassBuilder::Build()
{
	CompileDependencies();
	std::vector<RenderPassData> render_pass_order;
	render_pass_order.reserve(render_passes.size());
	std::stack<size_t> render_pass_dft_stack;
	std::vector<bool> visited = std::vector<bool>(render_passes.size(), false);
	std::vector<bool> visited_cycle = std::vector<bool>(render_passes.size(), false);
	
	if (root_passes.empty()) {
		for (int i = 0; i < render_passes.size(); i++) {
			TopologicalSortDFT(render_pass_dft_stack, i, visited, visited_cycle);
		}
	}
	else {
		for (auto root_pass : root_passes) {
			TopologicalSortDFT(render_pass_dft_stack, root_pass, visited, visited_cycle);
		}
	}
	while(!render_pass_dft_stack.empty()) {
		auto pass = render_pass_dft_stack.top();
		render_pass_dft_stack.pop();
		render_pass_order.push_back(RenderPassData{ render_passes[pass].render_pass, render_passes[pass].resources });
	}
	
	std::unordered_map<std::string, RenderPipeline::RenderPipelineResourceData_internal> resources;
	std::unordered_map<std::string, RenderPipeline::RenderPipelineResourceData_internal> out_persistent_resources;
	for (auto& resource_entry : resource_data) {
		resources.insert(std::make_pair(resource_entry.first, RenderPipeline::RenderPipelineResourceData_internal{nullptr,  resource_entry.second.desc }));
	}
	for (auto& persistent_resource_entry : persistent_resources) {
		out_persistent_resources.insert(std::make_pair(persistent_resource_entry.first, RenderPipeline::RenderPipelineResourceData_internal{ persistent_resource_entry.second.resource,  persistent_resource_entry.second.resource_desc }));
	}

	return RenderPipeline(std::move(render_pass_order), std::move(resources), std::move(out_persistent_resources));
}

void RenderPassBuilder::CompileDependencies()
{
	root_passes.clear();
	for (int i = 0; i < render_passes.size();i++) {
		auto& pass = render_passes[i];
		pass.dependencies.clear();
		bool has_dependencies = false;
		for (auto& resource : pass.resources.descriptors) {
			if (resource.desc_access == RenderPassResourceDescriptor_Access::READ) {
				auto fnd = resource_data.find(resource.resource_name);
				if (fnd == resource_data.end()) {
					throw std::runtime_error("Resource " + resource.resource_name + " could not be found");
				}
				auto& resource_entry = fnd->second;
				if (resource_entry.creator_index == -1) {
					throw std::runtime_error("Resource " + resource.resource_name + " is consumed but never created.");
				}
				has_dependencies = true;
				pass.dependencies.push_front(resource_entry.creator_index);
				render_passes[resource_entry.creator_index].depending_passes.push_front(i);
			}
		}
		if (!has_dependencies) {
			root_passes.push_back(i);
		}
	}
}

void RenderPassBuilder::TopologicalSortDFT(std::stack<size_t>& render_pass_dft_stack, size_t pass, std::vector<bool>& visited, std::vector<bool>& visited_cycle)
{
	if (visited[pass]) {
		if (visited_cycle[pass]) {
			throw std::runtime_error("RenderPipeline Cycle detected!");
		}
		return;
	}
	visited_cycle[pass] = true;
	visited[pass] = true;

	for (auto render_pass : render_passes[pass].depending_passes) {
		TopologicalSortDFT(render_pass_dft_stack, render_pass, visited, visited_cycle);
	}
	visited_cycle[pass] = false;
	render_pass_dft_stack.push(pass);
}
