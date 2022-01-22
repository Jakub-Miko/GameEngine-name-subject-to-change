#pragma once
#include <vector>
#include <Renderer/RendererDefines.h>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>
#include <Renderer/PipelineManager.h>
#include <Renderer/PipelinePresets.h>
#include <string>


struct RootDescriptorTableRange {
	RootDescriptorTableRange(RootDescriptorType type,uint32_t size) : type(type), size(size) {}
	RootDescriptorType type;
	uint32_t size;
};

using RootDescriptorTable = std::vector<RootDescriptorTableRange>;

struct RootSignatureDescriptorElement {
	RootSignatureDescriptorElement(const std::string& name, const RootDescriptorTable& table) : type(RootParameterType::DESCRIPTOR_TABLE), table(table), name(name) {}
	RootSignatureDescriptorElement(const std::string& name, RootDescriptorTable&& table) : type(RootParameterType::DESCRIPTOR_TABLE), table(std::move(table)), name(name) {}
	RootSignatureDescriptorElement(const std::string& name, RootParameterType type) : type(type), table(), name(name) {}
	RootParameterType type;
	std::string name;
	RootDescriptorTable table;
};

struct RootSignatureDescriptor {
	RootSignatureDescriptor(const std::vector<RootSignatureDescriptorElement>& parameters) : parameters(parameters) {}
	RootSignatureDescriptor(std::vector<RootSignatureDescriptorElement>&& parameters) : parameters(std::move(parameters)) {}
	std::vector<RootSignatureDescriptorElement> parameters;
};

class RootSignature {
public:
	using RootMappingTable = std::unordered_map<std::string, RootMappingEntry>;

	RootMappingEntry GetRootMapping(const std::string& semantic_name);

	static RootSignature* CreateSignature(const RootSignatureDescriptor& descriptor);
	virtual ~RootSignature() {}

protected:
	RootSignature() : RootMappings() {}
	RootSignature(const RootMappingTable& mapping) : RootMappings(mapping) {}
	RootMappingTable RootMappings;
};

//tends to be detected as a memory leak
template<typename T>
struct RootSignatureFactory {

	static RootSignature* GetRootSignature() {
		throw std::runtime_error("Not Implemented");
	}

};
//Tends to be detected as a memory leak
template<typename T>
struct VertexLayoutFactory {

	static VertexLayout* GetLayout() {
		throw std::runtime_error("Not Implemented");
	}

};



#pragma region PipelinePresets



class TestPreset { };

template<>
struct RootSignatureFactory<TestPreset> {

	static RootSignature* GetRootSignature() {
		static RootSignature* signature = nullptr;
		if (!signature) {
			RootSignature* sig = RootSignature::CreateSignature(RootSignatureDescriptor(
				{
					RootSignatureDescriptorElement("Testblock",RootParameterType::CONSTANT_BUFFER),
					RootSignatureDescriptorElement("TestTexture", RootParameterType::TEXTURE_2D)
				}
			));

			PipelineManager::Get()->AddSignature(sig);
			signature = sig;
		}

		return signature;
	}

};


template<>
struct VertexLayoutFactory<TestPreset> {

	static VertexLayout* GetLayout() {
		static VertexLayout* layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2)
				});

			PipelineManager::Get()->AddLayout(layout_new);

			layout = layout_new;
		}
		return layout;
	}

};
#pragma endregion
