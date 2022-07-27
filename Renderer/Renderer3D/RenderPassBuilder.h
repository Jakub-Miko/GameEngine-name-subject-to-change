#pragma once
#include "RenderPass.h"
#include <unordered_map>
#include <forward_list>
#include <stack>
#include "RenderPipeline.h"

struct RenderPassBuilder_Resource_data {
	RenderPassResourceDescriptor desc;
	size_t creator_index = (size_t)-1;
	std::forward_list<size_t> consumer_index_list;
};

struct RenderPassBuilder_RenderPass_data {
	std::shared_ptr<RenderPass> render_pass;
	RenderPassResourceDefinnition resources;
	std::forward_list<size_t> dependencies;
	std::forward_list<size_t> depending_passes;
};

class RenderPassBuilder {
public:
	RenderPassBuilder();

	void AddPass(RenderPass* render_pass);

	RenderPipeline Build();

private:
	void CompileDependencies();
	void TopologicalSortDFT(std::stack<size_t>& render_pass_dft_stack, size_t pass, std::vector<bool>& visited, std::vector<bool>& visited_cycle);

	std::unordered_map<std::string, RenderPassBuilder_Resource_data> resource_data;
	std::vector<RenderPassBuilder_RenderPass_data> render_passes;
	std::vector<size_t> root_passes;
};