#pragma once
#include <memory>
#include <Renderer/ShaderManager.h>
#include <Renderer/RenderResourceManager.h>
#include <vector>
#include <stdexcept>
#include <Renderer/RootSignature.h>
#include <stdint.h>

class Pipeline {
public:
	using BindingSlot = uint32_t;
	virtual ~Pipeline() = default;
};

struct VertexLayoutElement {
	RenderPrimitiveType type;
	int size;
};

using VertexLayout = std::vector<VertexLayoutElement>;

struct PipelineDescriptor {
	Shader* shader;
	VertexLayout vertex_layout;
	RootSignature* root_signature;
};

class DescriptorHeap {
public:
	virtual ~DescriptorHeap() {}
};

class DescriptorTable {
public:
	virtual ~DescriptorTable() {}
};


class PipelineManager {
public:
	
	static void Initialize();
	static PipelineManager* Get();
	static void Shutdown();

	virtual ~PipelineManager() {};
	virtual std::shared_ptr<Pipeline> CreatePipeline(const PipelineDescriptor& desc) = 0;

private:
	static PipelineManager* instance;

};