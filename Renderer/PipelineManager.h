#pragma once 
#include <Renderer/RendererDefines.h>
#include <Renderer/ShaderManager.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>

struct VertexLayout;
class RootSignature;

struct RenderViewport {
	RenderViewport(glm::vec2 position, glm::vec2 size,float min_depth, float max_depth) : position(position), size(size), min_depth(min_depth), max_depth(max_depth) {}
	glm::vec2 position;
	glm::vec2 size;
	float min_depth;
	float max_depth;
};


struct PipelineDescriptor {
	VertexLayout* layout;
	RootSignature* signature;
	Shader* shader;
};

class Pipeline {
public:
	virtual ~Pipeline() { }
	virtual RootBinding GetBindingId(const std::string& name) = 0;

	const VertexLayout& GetLayout() const {
		return *layout;
	}

	const RootSignature* GetSignature() const {
		return signature;
	}

	const Shader* GetShader() const {
		return shader;
	}

protected:
	Pipeline(const PipelineDescriptor& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader) {}
	Pipeline(PipelineDescriptor&& desc) : layout(desc.layout), signature(desc.signature), shader(desc.shader) {}
	VertexLayout* layout;
	RootSignature* signature;
	Shader* shader;
};


class PipelineManager {
public:
	
	template<typename U>
	friend struct VertexLayoutFactory;

	template<typename U>
	friend struct RootSignatureFactory;

	static void Initialize();
	static PipelineManager* Get();
	static void Shutdown();

	virtual Pipeline* CreatePipeline(const PipelineDescriptor& desc) = 0;
	virtual ~PipelineManager();

	PipelineManager();

private:
	static PipelineManager* instance;

private:
	std::vector<RootSignature*> m_Signatures;
	std::vector<VertexLayout> m_Layouts;

};
