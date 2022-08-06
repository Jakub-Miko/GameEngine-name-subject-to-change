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
	RootDescriptorTableRange() : type(RootDescriptorType::CONSTANT_BUFFER), size(0), name("Unknown"), individual_names() {}
	RootDescriptorTableRange(RootDescriptorType type,uint32_t size, std::string name) : type(type), size(size),name(name), individual_names() {}
	RootDescriptorType type;
	uint32_t size;
	std::string name;
	std::vector<std::string> individual_names;
};

struct ConstantBufferLayoutElement {
	RenderPrimitiveType type;
	std::string name;
};

using ConstantBufferLayout = std::vector<ConstantBufferLayoutElement>;
using RootDescriptorTable = std::vector<RootDescriptorTableRange>;

struct RootSignatureDescriptorElement {
	RootSignatureDescriptorElement(const std::string& name, const RootDescriptorTable& table, bool is_material_visible = false) : type(RootParameterType::DESCRIPTOR_TABLE), table(table), name(name), is_material_visible(is_material_visible) {}
	RootSignatureDescriptorElement(const std::string& name, RootDescriptorTable&& table, bool is_material_visible = false) : type(RootParameterType::DESCRIPTOR_TABLE), table(std::move(table)), name(name), is_material_visible(is_material_visible) {}
	RootSignatureDescriptorElement(const std::string& name, RootParameterType type, bool is_material_visible = false) : type(type), name(name),table(), is_material_visible(is_material_visible) {}
	RootSignatureDescriptorElement(const RootSignatureDescriptorElement& other) : type(other.type), name(other.name), table(other.table), is_material_visible(other.is_material_visible) {}
	RootSignatureDescriptorElement& operator=(const RootSignatureDescriptorElement& other) {
		type = other.type;
		name = other.name;
		table = other.table;
		is_material_visible = other.is_material_visible;
		return *this;
	}
	RootParameterType type;
	std::string name;
	RootDescriptorTable table;
	bool is_material_visible = false;
};

struct RootSignatureDescriptor {
	RootSignatureDescriptor() = default;
	RootSignatureDescriptor(const RootSignatureDescriptor& other) : parameters(other.parameters) {}
	RootSignatureDescriptor& operator=(const RootSignatureDescriptor& other) {
		parameters = other.parameters;
		return *this;
	}
	RootSignatureDescriptor(const std::vector<RootSignatureDescriptorElement>& parameters) : parameters(parameters) {}
	RootSignatureDescriptor(std::vector<RootSignatureDescriptorElement>&& parameters) : parameters(std::move(parameters)) {}
	std::vector<RootSignatureDescriptorElement> parameters;
};

class RootSignature {
public:
	using RootMappingTable = std::unordered_map<std::string, RootMappingEntry>;
	using ConstantBufferLayoutTable = std::unordered_map<std::string, ConstantBufferLayout>;
	RootMappingEntry GetRootMapping(const std::string& semantic_name) const;
	const ConstantBufferLayout& GetConstantBufferLayout(const std::string& semantic_name) const;

	const RootSignatureDescriptor& GetDescriptor() const {
		return descriptor;
	}

	static RootSignature* CreateSignature(const RootSignatureDescriptor& descriptor);
	static RootSignature* CreateSignature(const RootSignatureDescriptor& descriptor, RootMappingTable&& mapping_table, const ConstantBufferLayoutTable& const_buf_layouts = ConstantBufferLayoutTable());
	virtual ~RootSignature() {}

protected:
	RootSignature() : RootMappings(), ConstantBufferLayouts() {}
	RootSignature(const RootMappingTable& mapping, const ConstantBufferLayoutTable& const_buf_layouts = ConstantBufferLayoutTable()) : RootMappings(mapping), ConstantBufferLayouts(const_buf_layouts){}
	RootMappingTable RootMappings;
	ConstantBufferLayoutTable ConstantBufferLayouts;

	RootSignatureDescriptor descriptor;
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
