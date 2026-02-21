#include "ShaderManager.h"
#include <Renderer/MaterialManager.h>
#include <FileManager.h>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <Renderer/RootSignature.h>
#include <Core/UnitConverter.h>
#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLShaderManager.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanShaderManager.h>
#endif

NLOHMANN_JSON_SERIALIZE_ENUM(RootDescriptorType,
	{
	{RootDescriptorType::CONSTANT_BUFFER, "constant_buffer"},
	{RootDescriptorType::STORAGE_BUFFER, "storage_buffer"},
	{RootDescriptorType::TEXTURE_2D, "texture_2D"},
	{RootDescriptorType::TEXTURE_2D_ARRAY, "texture_2D_array"},
	{RootDescriptorType::TEXTURE_2D_CUBEMAP, "texture_2D_cubemap"}
	});

ShaderManager* ShaderManager::instance = nullptr;

void ShaderManager::Initialize()
{
	if (!instance) {
#ifdef OpenGL_API
		instance = new OpenGLShaderManager();
#elif defined Vulkan_API
		instance = new VulkanShaderManager();
#endif
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

std::shared_ptr<Shader> ShaderManager::GetShader(const std::string& path_in, const std::vector<std::string>& compiler_definitions)
{
	std::string path = FileManager::Get()->ResolvePath(FileManager::Get()->GetRenderApiAssetFilePath(path_in));
	std::lock_guard<std::mutex> lock(shader_map_mutex);

	std::string key_path = path;
	if(!compiler_definitions.empty()) {
		std::stringstream ss;
		ss << path << "#";
		for(auto& def : compiler_definitions) {
			ss << def << ";";
		}
		key_path = ss.str();
	}

	auto fnd = shader_map.find(key_path);
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
	Shader* shader = CreateShaderFromString_impl(shader_str, compiler_definitions);

	std::string root_sig_str;
	auto fnd_root = shader_str.find("#RootSignature");
	if (fnd_root == root_sig_str.npos) throw std::runtime_error("Shader file " + path + " does not contain RootSignatureDefinition");
	if (fnd_root != shader_str.npos) {
		auto end_root = shader_str.find("#end\n", fnd_root);
		fnd_root += strlen("#RootSignature");
		root_sig_str = shader_str.substr(fnd_root, end_root - fnd_root);
	};
	shader->signature = std::unique_ptr<RootSignature>(ParseRootSignature(root_sig_str));
	shader->path = path_in;
	std::shared_ptr<Shader> shader_out = std::shared_ptr<Shader>(shader);
	shader_map.insert(std::make_pair(key_path, shader_out));
	file_stream.close();
	return shader_out;
}

std::shared_ptr<Shader> ShaderManager::CreateShaderFromString(const std::string& shader_in, const std::vector<std::string>& compiler_definitions)
{
	Shader* shader = CreateShaderFromString_impl(shader_in, compiler_definitions);
	shader->path = "";
	std::string root_sig_str;
	auto fnd_root = shader_in.find("#RootSignature");
	if (fnd_root == root_sig_str.npos) throw std::runtime_error("Shader string does not contain RootSignatureDefinition");
	if (fnd_root != shader_in.npos) {
		auto end_root = shader_in.find("#end\n", fnd_root);
		fnd_root += strlen("#RootSignature");
		root_sig_str = shader_in.substr(fnd_root, end_root - fnd_root);
	}
	shader->signature = std::unique_ptr<RootSignature>(ParseRootSignature(root_sig_str));
	std::shared_ptr<Shader> shader_out = std::shared_ptr<Shader>(shader);
	return shader_out;
}



RootSignature* ShaderManager::ParseRootSignature(const std::string& signature_string)
{
	using namespace nlohmann;
	RootSignatureDescriptor desc;
	RootSignature::RootMappingTable mapping_table;
	json json_sig;
	json json_layouts;
	try {
		json json_object = json::parse(signature_string);

		json_sig = json_object["RootSignature"];
		if (!json_sig.is_array()) throw std::runtime_error("RootSignature json object isn't a list");
		int sig_entry_num = 0;

		for (auto& json_sig_element : json_sig) {
			std::string type = json_sig_element["type"].get<std::string>();
			std::string name = json_sig_element["name"].get<std::string>();

			if (type == "material") {
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
				desc.parameters.push_back(RootSignatureDescriptorElement(name,RootParameterType::MATERIAL));

				if (json_sig_element.contains("material_path")) {
					MaterialManager::Get()->LoadMaterialTemplateFile(json_sig_element["material_path"].get<std::string>());
				}

				if (json_sig_element.contains("material_inline")) {
					auto mat_template = MaterialManager::Get()->LoadMaterialTemplateFromJson(json_sig_element["material_inline"], name);
					MaterialManager::Get()->RegisterMaterialTemplate(mat_template);
				}

			}
			else if (type == "constant_buffer") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::CONSTANT_BUFFER));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
			}
			else if (type == "storage_buffer") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::STORAGE_BUFFER));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
			}
			else if (type == "texture_2D") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::TEXTURE_2D));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
			}
			else if (type == "texture_2D_array") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::TEXTURE_2D_ARRAY));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
			}
			else if (type == "texture_2D_cubemap") {
				desc.parameters.push_back(RootSignatureDescriptorElement(name, RootParameterType::TEXTURE_2D_CUBEMAP));
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
			} else if (type == "resource_store") {
				if(!json_sig_element.contains("store_type"))
					throw std::runtime_error("Resource store parameter must contain store_type");
				RootDescriptorType store_type = json_sig_element["store_type"].get<RootDescriptorType>();
				RootSignatureDescriptorElement element(name, RootParameterType::RESOURCE_STORE);
				element.store_type = store_type;
				desc.parameters.push_back(element);
				mapping_table.insert(std::make_pair(name, RootMappingEntry(sig_entry_num)));
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

	return RootSignature::CreateSignature(desc, std::move(mapping_table));


}

Shader::~Shader()
{

}
