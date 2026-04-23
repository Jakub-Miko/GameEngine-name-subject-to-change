#pragma once
#include <vector>
#include <Renderer/RendererDefines.h>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>
#include <Renderer/RenderResource.h>
#include "Renderer/MaterialManager.h"
#include <string>

#include "RenderResourceStore.h"


//struct RootDescriptorTableRange {
//	RootDescriptorTableRange() : type(RootDescriptorType::CONSTANT_BUFFER), size(0), name("Unknown"), individual_names() {}
//	RootDescriptorTableRange(RootDescriptorType type,uint32_t size, std::string name) : type(type), size(size),name(name), individual_names() {}
//	RootDescriptorType type;
//	uint32_t size;
//	std::string name;
//	std::vector<std::string> individual_names;
//};

//struct ConstantBufferLayoutElement {
//	RenderPrimitiveType type;
//	std::string name;
//};

//using ConstantBufferLayout = std::vector<ConstantBufferLayoutElement>;
//using RootDescriptorTable = std::vector<RootDescriptorTableRange>;


struct RootSignatureDescriptorResource {
	uint32_t binding_id = 0;
};

struct RootSignatureDescriptorMaterial {
	std::shared_ptr<MaterialTemplate> material_template = nullptr;
	uint32_t set_id;
};

struct RootSignatureDescriptorResourceStore {
	RootDescriptorType store_type = RootDescriptorType::TEXTURE_2D;
	uint32_t set_id;
};


class RootSignatureDescriptorElement {
public:
	RootSignatureDescriptorElement() = default;
	RootSignatureDescriptorElement(const RootSignatureDescriptorElement& other) = default;

	static RootSignatureDescriptorElement CreateResourceElement(const std::string& name, RootParameterType type) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = type;
		element.element_info = RootSignatureDescriptorResource { 0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateConstantBufferElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::CONSTANT_BUFFER;
		element.element_info = RootSignatureDescriptorResource{ 0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateStorageBufferElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::STORAGE_BUFFER;
		element.element_info = RootSignatureDescriptorResource{ 0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateTexture2DElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::TEXTURE_2D;
		element.element_info = RootSignatureDescriptorResource{ 0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateTexture2DArrayElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::TEXTURE_2D_ARRAY;
		element.element_info = RootSignatureDescriptorResource{ 0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateTexture2DCubemapElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::TEXTURE_2D_CUBEMAP;
		element.element_info = RootSignatureDescriptorResource{ 0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateMaterialElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::MATERIAL;
		element.element_info = RootSignatureDescriptorMaterial{ nullptr,0 };
		return element;
	}

	static RootSignatureDescriptorElement CreateResourceStoreElement(const std::string& name, RootDescriptorType store_type) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::RESOURCE_STORE;
		element.element_info = RootSignatureDescriptorResourceStore{ store_type };
		return element;
	}

	static RootSignatureDescriptorElement CreateStorageTextureElement(const std::string& name) {
		RootSignatureDescriptorElement element;
		element.name = name;
		element.type = RootParameterType::STORAGE_TEXTURE;
		element.element_info = RootSignatureDescriptorResource { 0 };
		return element;
	}

	RootSignatureDescriptorResource& GetResourceInfo() {
		return std::get<RootSignatureDescriptorResource>(element_info);
	}

	RootSignatureDescriptorMaterial& GetMaterialInfo() {
		return std::get<RootSignatureDescriptorMaterial>(element_info);
	}

	RootSignatureDescriptorResourceStore& GetResourceStoreInfo() {
		return std::get<RootSignatureDescriptorResourceStore>(element_info);
	}

	RootParameterType GetType() const {
		return type;
	}

	const std::string& GetName() const {
		return name;
	}

private:
	std::string name = "";
	RootParameterType type = RootParameterType::UNDEFINED;
	std::variant<RootSignatureDescriptorResource, RootSignatureDescriptorMaterial, RootSignatureDescriptorResourceStore> element_info;
};

struct RootSignatureDescriptor {
	RootSignatureDescriptor() = default;
	RootSignatureDescriptor(const RootSignatureDescriptor& other) : parameters(other.parameters), push_constant_range_size(other.push_constant_range_size) {}
	RootSignatureDescriptor& operator=(const RootSignatureDescriptor& other) {
		parameters = other.parameters;
		return *this;
	}
	RootSignatureDescriptor(const std::vector<RootSignatureDescriptorElement>& parameters) : parameters(parameters) {}
	RootSignatureDescriptor(std::vector<RootSignatureDescriptorElement>&& parameters) : parameters(std::move(parameters)) {}
	std::vector<RootSignatureDescriptorElement> parameters;
	uint32_t push_constant_range_size = 0;
};

struct RootMappingEntry {
	RootMappingEntry() : parameter_id(0) {}
	RootMappingEntry(uint32_t parameter_id) : parameter_id(parameter_id) {}
	uint32_t parameter_id = 0;
};

class RootSignature {
public:
	using RootMappingTable = std::unordered_map<std::string, RootMappingEntry>;
	RootMappingEntry GetRootParameterId(const std::string& semantic_name) const;
	const RootSignatureDescriptorElement& GetRootParameter(const std::string& semantic_name) const;

	const RootSignatureDescriptor& GetDescriptor() const {
		return descriptor;
	}

	static RootSignature* CreateSignature(const RootSignatureDescriptor& descriptor);
	static RootSignature* CreateSignature(const RootSignatureDescriptor& descriptor, RootMappingTable&& mapping_table);
	virtual ~RootSignature() {}

protected:
	RootSignature() : RootMappings() {}
	RootSignature(const RootMappingTable& mapping) : RootMappings(mapping) {}
	RootSignature(const RootSignatureDescriptor& descriptor) : descriptor(descriptor) {}
	RootMappingTable RootMappings;
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
				// {

				// 	RootSignatureDescriptorElement("Test",RootDescriptorTable({
				// 		RootDescriptorTableRange(RootDescriptorType::CONSTANT_BUFFER,1,"Testblock"),
				// 		RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D, 1, "TestTexture")
				// 		}))
				// }
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
					RootSignatureDescriptorElement::CreateConstantBufferElement("conf")
				}
			));
			signature = sig;
		}

		return signature;
	}

};

struct SkeletalGeometryPassPreset;

template<>
struct VertexLayoutFactory<SkeletalGeometryPassPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::UNSIGNED_INT,4,"bone_ids"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4,"bone_weights"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "normal"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4, "tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv0")
				});


			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
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
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4,"tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2,"uv0")
				});

			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};

class SkeletalMeshPreset { };

template<>
struct VertexLayoutFactory<SkeletalMeshPreset> {

	static VertexLayout* GetLayout() {
		static std::unique_ptr<VertexLayout> layout = nullptr;
		if (!layout) {
			VertexLayout* layout_new = new VertexLayout({
				VertexLayoutElement(RenderPrimitiveType::UNSIGNED_INT,4,"bone_ids"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4,"bone_weights"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "position"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,3, "normal"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,4, "tangent"),
				VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv0")
				});

			layout = std::unique_ptr<VertexLayout>(layout_new);
		}
		return layout.get();
	}

};
#pragma endregion


#pragma endregion
