#include "ShaderManager.h"
#include <Renderer/Renderer3D/MaterialManager.h>
#include <FileManager.h>
#include <platform/OpenGL/OpenGLShaderManager.h>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <Renderer/RootSignature.h>
#include <Core/UnitConverter.h>

ShaderManager* ShaderManager::instance = nullptr;

void ShaderManager::Initialize()
{
	if (!instance) {
		instance = new OpenGLShaderManager();
	}
}

ShaderManager* ShaderManager::Get()
{
	return instance;
}

void ShaderManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

ShaderManager::~ShaderManager()
{
}

std::shared_ptr<Shader> ShaderManager::GetShader(const std::string& path_in)
{
	std::string path = FileManager::Get()->ResolvePath(FileManager::Get()->GetRenderApiAssetFilePath(path_in));
	std::lock_guard<std::mutex> lock(shader_map_mutex);
	auto fnd = shader_map.find(path);
	if (fnd != shader_map.end()) {
		return fnd->second;
	}
	std::ifstream file_stream(path);
	if (!file_stream.is_open()) {
		throw std::runtime_error("File " + path + " could not be opened");
	}
	std::stringstream s_stream;
	s_stream << file_stream.rdbuf();
	std::string shader_str = s_stream.str();
	Shader* shader = CreateShaderFromString_impl(shader_str);

	std::string root_sig_str;
	auto fnd_root = shader_str.find("#RootSignature");
	if (fnd_root == root_sig_str.npos) throw std::runtime_error("Shader file " + path + " does not contain RootSignatureDefinition");
	if (fnd_root != shader_str.npos) {
		auto end_root = shader_str.find("#end", fnd_root);
		fnd_root += strlen("#RootSignature");
		root_sig_str = shader_str.substr(fnd_root, end_root - fnd_root);
	}
	bool has_default_material;
	shader->signature = std::unique_ptr<RootSignature>(ParseRootSignature(root_sig_str, &has_default_material));
	shader->path = path_in;
	std::shared_ptr<Shader> shader_out = std::shared_ptr<Shader>(shader);
	if (has_default_material) {
		shader_out->default_material = ParseDefaulMaterial(root_sig_str, shader_out);
	}
	shader_map.insert(std::make_pair(path, shader_out));
	file_stream.close();
	return shader_out;
}

std::shared_ptr<Shader> ShaderManager::CreateShaderFromString(const std::string& shader_in)
{
	Shader* shader = CreateShaderFromString_impl(shader_in);
	shader->path = "";
	std::string root_sig_str;
	auto fnd_root = shader_in.find("#RootSignature");
	if (fnd_root == root_sig_str.npos) throw std::runtime_error("Shader string does not contain RootSignatureDefinition");
	if (fnd_root != shader_in.npos) {
		auto end_root = shader_in.find("#end", fnd_root);
		fnd_root += strlen("#RootSignature");
		root_sig_str = shader_in.substr(fnd_root, end_root - fnd_root);
	}
	bool has_default_material;
	shader->signature = std::unique_ptr<RootSignature>(ParseRootSignature(root_sig_str, &has_default_material));
	std::shared_ptr<Shader> shader_out = std::shared_ptr<Shader>(shader);
	if (has_default_material) {
		shader_out->default_material = ParseDefaulMaterial(root_sig_str, shader_out);
	}
	return shader_out;
}



RootSignature* ShaderManager::ParseRootSignature(const std::string& signature_string, bool* has_default_material)
{
	using namespace nlohmann;
	RootSignatureDescriptor desc;
	RootSignature::RootMappingTable mapping_table;
	RootSignature::ConstantBufferLayoutTable layout_table;
	json json_sig;
	json json_layouts;
	try {
		json json_object = json::parse(signature_string);

		if (has_default_material) {
			*has_default_material = json_object.contains("default_material");
		}

		if (json_object.contains("constant_buffer_layouts")) {
			if (!json_object["constant_buffer_layouts"].is_array()) throw std::runtime_error("constant_buffer_layouts must be an array");
			for (auto& const_layout : json_object["constant_buffer_layouts"]) {
				std::string name = const_layout["name"].get<std::string>();
				if (!const_layout.contains("layout") && !const_layout["layout"].is_array()) throw std::runtime_error("constant_buffer_layouts item must contain a list called layout");
				ConstantBufferLayout const_layout_object;
				for (auto& layout_item : const_layout["layout"]) {
					ConstantBufferLayoutElement element;
					element.name = layout_item["name"].get<std::string>();
					element.type = layout_item["type"].get<RenderPrimitiveType>();
					const_layout_object.push_back(element);
				}
				layout_table.insert(std::make_pair(name, const_layout_object));
			}

		}

		json_sig = json_object["RootSignature"];
		if (!json_sig.is_array()) throw std::runtime_error("RootSignature json object isn't a list");
		int sig_entry_num = 0;

		for (auto& json_sig_element : json_sig) {
			std::string type = json_sig_element["type"].get<std::string>();
			std::string name = json_sig_element["name"].get<std::string>();
			bool is_material_visible = false;
			if (json_sig_element.contains("material_visible") && json_sig_element["material_visible"].is_boolean() && json_sig_element["material_visible"].get<bool>()) {
				is_material_visible = true;
			}

			if (type == "descriptor_table") {
				RootDescriptorTable table;
				int table_item_num = 0;
				for (auto& range : json_sig_element["ranges"]) {
					RootDescriptorTableRange range_object;
					uint32_t range_size = range["size"].get<uint32_t>();
					std::string range_type = range["type"].get<std::string>();
					std::string range_name = range["name"].get<std::string>();
					range_object.size = range_size;
					range_object.name = range_name;
					if (range_type == "constant_buffer") {
						range_object.type = RootDescriptorType::CONSTANT_BUFFER;
					}
					if (range_type == "texture_2D") {
						range_object.type = RootDescriptorType::TEXTURE_2D;
					}
					if (range_type == "texture_2D_array") {
						range_object.type = RootDescriptorType::TEXTURE_2D_ARRAY;
					}
					if (range.contains("individual_names")) {
						if (!range["individual_names"].is_array()) throw std::runtime_error("individual_names must be a list of strings");
						if (range["individual_names"].size() != range_size) throw std::runtime_error("individual_names must be the same length as range size");
						range_object.individual_names = range["individual_names"].get<std::vector<std::string>>();
						for (auto& table_item_name : range_object.individual_names) {
							RootParameterType type;
							if (range_object.type == RootDescriptorType::CONSTANT_BUFFER) {
								type = RootParameterType::CONSTANT_BUFFER;
							}
							else if (range_object.type == RootDescriptorType::TEXTURE_2D) {
								type = RootParameterType::TEXTURE_2D;
							}
							else if (range_object.type == RootDescriptorType::TEXTURE_2D_ARRAY) {
								type = RootParameterType::TEXTURE_2D_ARRAY;
							}
							else {
								type = RootParameterType::UNDEFINED;
							}
							mapping_table.insert(std::make_pair(table_item_name, RootMappingEntry(table_item_num, type, sig_entry_num)));
							table_item_num++;
						}
					}
					else {
						table_item_num += range_size;
					}
					table.push_back(range_object);
					mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num, RootParameterType::DESCRIPTOR_TABLE)));
				}
				desc.parameters.push_back(RootSignatureDescriptorElement(name, table, is_material_visible));
			}
			else if (type == "constant_buffer") {
				if (json_sig_element.contains("layout")) {
					auto fnd = layout_table.find(json_sig_element["layout"].get<std::string>());
					if (fnd != layout_table.end()) {
						layout_table.insert(std::make_pair(name, fnd->second));
					}
					else {
						throw std::runtime_error("layout of name " + json_sig_element["layout"].get<std::string>() + " doesn't exist");
					}
				}

				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::CONSTANT_BUFFER, is_material_visible));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num, RootParameterType::CONSTANT_BUFFER)));
			}
			else if (type == "texture_2D") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::TEXTURE_2D, is_material_visible));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num, RootParameterType::TEXTURE_2D)));
			}
			else if (type == "texture_2D_array") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::TEXTURE_2D_ARRAY, is_material_visible));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num, RootParameterType::TEXTURE_2D_ARRAY)));
			}
			else {
				throw std::runtime_error("Root Signature parameter type: " + type + " isn't supported");
			}
			sig_entry_num++;
		}
	}
	catch (...) {
		throw std::runtime_error("Could not parse the Root Signature");
	}

	return RootSignature::CreateSignature(desc, std::move(mapping_table), layout_table);


}

std::shared_ptr<Material> ShaderManager::ParseDefaulMaterial(const std::string& signature_string, std::shared_ptr<Shader> shader)
{
	using namespace nlohmann;
	json json_object = json::parse(signature_string);

	if (json_object.contains("default_material")) {
		return MaterialManager::Get()->ParseMaterialFromString(json_object["default_material"].dump(), shader);
	}

}

Shader::~Shader()
{

}
