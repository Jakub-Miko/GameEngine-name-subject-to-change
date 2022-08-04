#pragma once
#include <vector>
#include <Renderer/RendererDefines.h>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>
#include <Renderer/PipelinePresets.h>
#include <Renderer/RenderResource.h>
#include <string>


struct RootDescriptorTableRange {
	RootDescriptorTableRange() : type(RootDescriptorType::CONSTANT_BUFFER), size(0), name("Unknown") {}
	RootDescriptorTableRange(RootDescriptorType type,uint32_t size, std::string name) : type(type), size(size),name(name) {}
	RootDescriptorType type;
	uint32_t size;
	std::string name;
};

using RootDescriptorTable = std::vector<RootDescriptorTableRange>;

struct RootSignatureDescriptorElement {
	RootSignatureDescriptorElement(const std::string& name, const RootDescriptorTable& table) : type(RootParameterType::DESCRIPTOR_TABLE), table(table), name(name) {}
	RootSignatureDescriptorElement(const std::string& name, RootDescriptorTable&& table) : type(RootParameterType::DESCRIPTOR_TABLE), table(std::move(table)), name(name) {}
	RootSignatureDescriptorElement(const std::string& name, RootParameterType type) : type(type), name(name),table() {}
	const RootParameterType type;
	std::string name;
	RootDescriptorTable table;
};

struct RootSignatureDescriptor {
	RootSignatureDescriptor() = default;
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

#pragma region TestPreset



class TestPreset { };

template<>
struct RootSignatureFactory<TestPreset> {

	static RootSignature* GetRootSignature() {
		static RootSignature* signature = nullptr;
		if (!signature) {
			RootSignature* sig = RootSignature::CreateSignature(RootSignatureDescriptor(
				{

					RootSignatureDescriptorElement("Test",RootDescriptorTable({
						RootDescriptorTableRange(RootDescriptorType::CONSTANT_BUFFER,1,"Testblock"),
						RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D, 1, "TestTexture")
						}))
				}
			));

			signature = sig;
		}

		return signature;
	}

};


template<>
struct VertexLayoutFactory<TestPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "normal")
				});


			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

#pragma endregion

#pragma region BoxPreset
class BoxPreset { };

template<>
struct RootSignatureFactory<BoxPreset> {

	static RootSignature* GetRootSignature() {
		static RootSignature* signature = nullptr;
		if (!signature) {
			RootSignature* sig = RootSignature::CreateSignature(RootSignatureDescriptor(
				{
					RootSignatureDescriptorElement("conf",RootParameterType::CONSTANT_BUFFER)
				}
			));
			signature = sig;
		}

		return signature;
	}

};


template<>
struct VertexLayoutFactory<BoxPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4,"position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4,"normal")
				});

			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

class MeshPreset { };

template<>
struct VertexLayoutFactory<MeshPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3,"position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3,"normal"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3,"tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2,"uv0")
				});

			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};
#pragma endregion


#pragma endregion
