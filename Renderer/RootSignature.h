#pragma once
#include <vector>
#include <Renderer/RendererDefines.h>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>
#include <Renderer/RendererDefines.h>
#include <Renderer/PipelinePresets.h>
#include <string>

using RootBinding = unsigned int;

enum class RootParameterType : unsigned char {
	UNDEFINED = 0, CONSTANT_BUFFER = 1, TEXTURE_2D = 2, DESCRIPTOR_TABLE = 3
};

enum class RootDescriptorType : unsigned char {
	CONSTANT_BUFFER = 0, TEXTURE_2D = 1
};

struct RootMappingEntry {
	RootMappingEntry() : binding_id(0), type(RootParameterType::UNDEFINED) {}
	RootMappingEntry(RootBinding binding_id, RootParameterType type) : binding_id(binding_id), type(type) {}
	RootBinding binding_id;
	RootParameterType type;
};

struct VertexLayoutElement {
	VertexLayoutElement(RenderPrimitiveType type, uint32_t size) : type(type), size(size) {}
	RenderPrimitiveType type;
	uint32_t size;
};

struct VertexLayout {
	
	VertexLayout() : layout(), stride(0) {}

	VertexLayout(const std::vector<VertexLayoutElement>& layout) : layout(layout), stride(0) {
		CalculateStride();
	}

	VertexLayout(std::vector<VertexLayoutElement>&& layout) : layout(std::move(layout)), stride(0) {
		CalculateStride();
	}

	VertexLayout(const VertexLayout& layout) : layout(layout.layout), stride(layout.stride) {

	}

	VertexLayout(VertexLayout&& layout) noexcept : layout(std::move(layout.layout)), stride(layout.stride) {

	}

	VertexLayout& operator=(const VertexLayout& ref) {
		layout = ref.layout;
		stride = ref.stride;
		return* this;
	}
	
	VertexLayout& operator=(VertexLayout&& ref) {
		layout = std::move(ref.layout);
		stride = ref.stride;
		return *this;
	}

	std::vector<VertexLayoutElement> layout;
	int stride;
private:

	void CalculateStride();

};


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

protected:
	RootSignature() : RootMappings() {}
	RootSignature(const RootMappingTable& mapping) : RootMappings(mapping) {}
	virtual ~RootSignature() {}
	RootMappingTable RootMappings;
};


template<typename T>
struct RootSignatureFactory {

	static RootSignature* GetRootSignature() {
		throw std::runtime_error("Not Implemented");
	}

};

template<typename T>
struct VertexLayoutFactory {

	static VertexLayout& GetLayout() {
		throw std::runtime_error("Not Implemented");
	}

};

#pragma region PipelinePresets



class TestPreset { };

template<>
struct RootSignatureFactory<TestPreset> {

	static RootSignature* GetRootSignature() {
		static RootSignature* signature = RootSignature::CreateSignature(RootSignatureDescriptor(
			{
				RootSignatureDescriptorElement("Testblock",RootParameterType::CONSTANT_BUFFER),
			}
		));
		return signature;
	}

};


template<>
struct VertexLayoutFactory<TestPreset> {

	static VertexLayout& GetLayout() {
		static VertexLayout layout = VertexLayout({
			VertexLayoutElement(RenderPrimitiveType::FLOAT,2)
			});
		return layout;
	}

};
#pragma endregion
