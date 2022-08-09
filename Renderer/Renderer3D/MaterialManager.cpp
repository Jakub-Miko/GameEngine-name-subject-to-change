#include "MaterialManager.h"
#include <Core/UnitConverter.h>
#include <Renderer/TextureManager.h>
#include <Renderer/RenderResourceManager.h>
#include <set>
#include <algorithm>
#include <FileManager.h>
#include <fstream>
#include <sstream>
#include <json.hpp>
#include <Renderer/ShaderManager.h>

MaterialManager* MaterialManager::instance = nullptr;

void MaterialManager::Init()
{
	if (!instance) {
		instance = new MaterialManager;
	}
}

void MaterialManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

MaterialManager* MaterialManager::Get()
{
	return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string& path_in)
{
	std::lock_guard<std::mutex> lock(material_mutex);
	std::string path = FileManager::Get()->GetPath(path_in);
	auto fnd = materials.find(path);
	if (fnd != materials.end()) {
		return fnd->second;
	}
	auto material = ParseMaterialFromFile(path);
	materials.insert(std::make_pair(path, material));
	return material;
}

MaterialManager::MaterialManager() : materials(), material_templates(), material_mutex(), material_load(), material_load_mutex()
{

}

std::shared_ptr<Material> MaterialManager::ParseMaterialFromFile(const std::string& path)
{
	using namespace nlohmann;
	std::ifstream stream(path);
	if (!stream.is_open()) throw std::runtime_error("File" + path + " could not be opened");
	std::stringstream sstream;
	sstream << stream.rdbuf();
	stream.close();
	auto mat = ParseMaterialFromString(sstream.str());
	mat->material_path = path;
	return mat;
}

std::shared_ptr<Material> MaterialManager::ParseMaterialFromString(const std::string& string, std::shared_ptr<Shader> shader_spec)
{
	using namespace nlohmann;
	json material_json = json::parse(string);
	std::shared_ptr<MaterialTemplate> mat_template;
	if (shader_spec == nullptr) {
		std::string shader_path = material_json["shader"].get<std::string>();
		auto fnd_template = material_templates.find(shader_path);
		if (fnd_template != material_templates.end()) {
			mat_template = fnd_template->second;
		}
		else {
			mat_template = std::make_shared<MaterialTemplate>(ShaderManager::Get()->GetShader(shader_path));
			material_templates.insert(std::make_pair(shader_path, mat_template));
		}
	}
	else {

		//If we explicitly specify the shader the lifetime need to be manager by the function.
		mat_template = std::make_shared<MaterialTemplate>(shader_spec);
	}

	std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material(mat_template));
	if (!material_json.contains("parameters") || !material_json["parameters"].is_array()) throw std::runtime_error("Material file must contain a parameters list");
	for (auto& parameter : material_json["parameters"]) {
		std::string type = parameter["type"].get<std::string>();
		if (type == "SCALAR") {
			material->SetParameter<float>(parameter["name"].get<std::string>(), parameter["value"].get<float>());
		}
		else if (type == "VEC2") {
			material->SetParameter<glm::vec2>(parameter["name"].get<std::string>(), parameter["value"].get<glm::vec2>());
		}
		else if (type == "VEC3") {
			material->SetParameter<glm::vec3>(parameter["name"].get<std::string>(), parameter["value"].get<glm::vec3>());
		}
		else if (type == "VEC4") {
			material->SetParameter<glm::vec4>(parameter["name"].get<std::string>(), parameter["value"].get<glm::vec4>());
		}
		else if (type == "TEXTURE") {
			material->SetParameter(parameter["name"].get<std::string>(), TextureManager::Get()->GetDefaultTexture());
			if (parameter["value"].get<std::string>().empty()) continue;
			material->SetTexture(parameter["name"].get<std::string>(), parameter["value"].get<std::string>());
		}
	}
	material->material_path = "";
	return material;
}

void Material::SetMaterial(RenderCommandList* command_list, std::shared_ptr<Pipeline> pipeline)
{
	UpdateValues(command_list);
	auto& sig = material_template->GetRootSignature().GetDescriptor().parameters;
	for (auto& resource : resources) {
		if (resource.is_table) {
			command_list->SetDescriptorTable(sig[resource.index].name, std::get<RenderDescriptorTable>(resource.resource));
		}
		else {
			command_list->SetConstantBuffer(sig[resource.index].name, std::get<std::shared_ptr<RenderBufferResource>>(resource.resource));
		}
	}
	for (auto& parameter : parameters) {
		if (parameter.type == MaterialTemplate::MaterialTemplateParameterType::TEXTURE && !parameter.table) {
			command_list->SetTexture2D(parameter.name, std::get<std::shared_ptr<RenderTexture2DResource>>(parameter.resource));
		}
	}

}

void Material::UpdateValues(RenderCommandList* command_list)
{
	for (auto& parameter : parameters) {
		if(parameter.dirty) {
			if (parameter.type == MaterialTemplate::TEXTURE) {
				auto& texture_param_info = material_template->GetMaterialTemplateParameter(parameter.name);
				if (texture_param_info.descriptor_table_id != -1) {
					auto res_fnd = std::find_if(resources.begin(), resources.end(), [&texture_param_info](const MaterialResource& res) {return res.index == texture_param_info.descriptor_table_id; });
					if (res_fnd != resources.end()) {
						RenderResourceManager::Get()->CreateTexture2DDescriptor(std::get<RenderDescriptorTable>(res_fnd->resource),
							texture_param_info.index, std::get<std::shared_ptr<RenderTexture2DResource>>(parameter.resource));
						parameter.dirty = false;
					}
					else {
						throw std::runtime_error("Descriptor table for parameter " + parameter.name + " not found");
					}
				}
			}
			else {
				auto& param_info = material_template->GetMaterialTemplateParameter(parameter.name);
				auto res_fnd = std::find_if(resources.begin(), resources.end(), [&param_info](const MaterialResource& res) {return res.index == param_info.index; });
				if(res_fnd != resources.end()) {
					RenderResourceManager::Get()->UploadDataToBuffer(command_list, std::get<std::shared_ptr<RenderBufferResource>>(res_fnd->resource),
						&parameter.resource, param_info.primitive_size, param_info.constant_buffer_offset);
					parameter.dirty = false;
				}
				else {
					throw std::runtime_error("Constant Buffer for parameter " + parameter.name + " not found");
				}
			}
		}
	}
}

void Material::SetTexture(const std::string& name, const std::string& path)
{
	if (TextureManager::Get()->IsTextureAvailable(path)) {
		SetParameter(name, TextureManager::Get()->LoadTextureFromFile(path, false));
	}
	else {
		auto future = TextureManager::Get()->LoadTextureFromFileAsync(path, true);
		MaterialManager::Get()->AddTextureLoad(shared_from_this(),name, future);
	}
}

Material::Material(std::shared_ptr<MaterialTemplate> material_template) : material_template(material_template), parameters(), resources()
{
	for (auto& material_param : material_template->GetMaterialTemplateParameters()) {
		MaterialParameter res;
		res.dirty = true;
		res.name = material_param.name;
		res.type = material_param.type;
		res.table = material_param.descriptor_table_id != -1;
		switch (res.type)
		{
		case MaterialTemplate::MaterialTemplateParameterType::SCALAR:
			res.resource = 1.0f;
			break;
		case MaterialTemplate::MaterialTemplateParameterType::VEC2:
			res.resource = glm::vec2(1.0f);
			break;
		case MaterialTemplate::MaterialTemplateParameterType::VEC3:
			res.resource = glm::vec3(1.0f);
			break;
		case MaterialTemplate::MaterialTemplateParameterType::VEC4:
			res.resource = glm::vec4(1.0f);
			break;
		case MaterialTemplate::MaterialTemplateParameterType::TEXTURE:
			res.resource = TextureManager::Get()->GetDefaultTexture();
			break;
		default:
			throw std::runtime_error("Unsupported tempate parameter type " + material_param.name);
			break;
		}
		parameters.push_back(res);
	}
	std::set<int> const_buf_index;
	std::set<int> table_index;
	const auto& signature = material_template->GetRootSignature();
	for (auto& param_types : material_template->GetMaterialTemplateParameters()) {
		if (param_types.constant_buffer_offset != -1) {
			if (const_buf_index.insert(param_types.index).second) {
				int size = material_template->GetTableOrBufferSize(param_types.index);
				MaterialResource res;
				res.index = param_types.index;
				res.is_table = false;
				RenderBufferDescriptor desc(size, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
				res.resource = RenderResourceManager::Get()->CreateBuffer(desc);
				resources.push_back(res);
			}
		}
		else if (param_types.descriptor_table_id != -1) {
			if (table_index.insert(param_types.descriptor_table_id).second) {
				int size = material_template->GetTableOrBufferSize(param_types.descriptor_table_id);
				MaterialResource res;
				res.index = param_types.descriptor_table_id;
				res.is_table = true;
				res.resource = Renderer3D::Get()->GetDescriptorHeap().Allocate(size);
				resources.push_back(res);
			}
		}
	}

}

MaterialTemplate::MaterialTemplate(std::shared_ptr<Shader> shader_in) : material_parameters(), material_parameters_map(), shader_wk(shader_in), buffer_and_descriptor_table_sizes()
{
	auto shader = GetShader();
	auto& desc_parametrs = shader->GetRootSignature().GetDescriptor().parameters;
	int index = 0;
	for (auto& parameter : desc_parametrs) {
		if (parameter.is_material_visible) {
			switch (parameter.type)
			{
			case RootParameterType::TEXTURE_2D:
				AddTexture2DParameter(parameter,index);
				break;
			case RootParameterType::CONSTANT_BUFFER:
				AddConstantBufferParameter(parameter, index);
				break;
			case RootParameterType::DESCRIPTOR_TABLE:
				AddDescriptorTableParameter(parameter, index);
				break;
			default:
				break;
			}
		}
		index++;
	}
}

const MaterialTemplate::MaterialTemplateParameter& MaterialTemplate::GetMaterialTemplateParameter(const std::string& name) const
{
	auto fnd = material_parameters_map.find(name);
	if (fnd != material_parameters_map.end()) {
		return material_parameters[fnd->second];
	}
	else {
		throw std::runtime_error("Material template parameter " + name + " not found");
	}
}

MaterialTemplate::~MaterialTemplate()
{

}

int MaterialTemplate::GetTableOrBufferSize(int index)
{
	auto fnd = buffer_and_descriptor_table_sizes.find(index);
	if (fnd != buffer_and_descriptor_table_sizes.end()) {
		return fnd->second;
	}
	else {
		throw std::runtime_error("Table or buffer size " + std::to_string(index) + " not found");
	}
	
}

void MaterialTemplate::CreateParameter(const MaterialTemplateParameter& parameter)
{
	material_parameters.push_back(parameter);
	material_parameters_map.insert(std::make_pair(parameter.name,material_parameters.size()-1));
}

const RootSignature& MaterialTemplate::GetRootSignature() const {
	return GetShader()->GetRootSignature();
}

void MaterialTemplate::AddTexture2DParameter(const RootSignatureDescriptorElement& element, int index, uint32_t table )
{
	MaterialTemplateParameter parameter;
	if (table != -1) {
		parameter.descriptor_table_id = table;
		parameter.index = index;
		parameter.name = element.name;
		parameter.type = TEXTURE;
	}
	else {
		parameter.index = index;
		parameter.name = element.name;
		parameter.type = TEXTURE;
	}
	CreateParameter(parameter);
}

void MaterialTemplate::AddDescriptorTableParameter(const RootSignatureDescriptorElement& element, int index)
{
	int current_index = 0;
	for (auto& range : element.table) {
		switch (range.type) {
		//Descriptor table contained Constant buffer dont support MaterialParameters, since their layout can be defined at runtime.
		case RootDescriptorType::TEXTURE_2D:
			if (range.individual_names.empty()) {
				throw std::runtime_error("Material Visible Textures in Descriptor tables must have assigned individual names.");
			}
			else {
				for (int i = 0; i < range.size; i++) {
					AddTexture2DParameter(RootSignatureDescriptorElement(range.individual_names[i], RootParameterType::TEXTURE_2D, true),current_index,index);
					current_index++;
				}
			}
			break;
		default:
			current_index += range.size;
			break;
		}
	}
	buffer_and_descriptor_table_sizes.insert(std::make_pair(index, current_index));
}

void MaterialTemplate::AddConstantBufferParameter(const RootSignatureDescriptorElement& element, int index, uint32_t table)
{
	auto shader = GetShader();
	try {
		auto layout = shader->GetRootSignature().GetConstantBufferLayout(element.name);
		int offset = 0;
		for (auto& entry : layout) {
			MaterialTemplateParameter param;
			param.constant_buffer_offset = offset;
			if (table == -1) {
				param.index = index;
				param.descriptor_table_id = -1;
			}
			else {
				throw std::runtime_error("Descriptor table contained Constant buffer dont support MaterialParameters, since their layout can be defined at runtime.");
			}
			param.name = entry.name;
			switch (entry.type) {
			case RenderPrimitiveType::FLOAT:
				param.type = SCALAR;
				break;
			case RenderPrimitiveType::VEC2:
				param.type = VEC2;
				break;
			case RenderPrimitiveType::VEC3:
				param.type = VEC3;
				break;
			case RenderPrimitiveType::VEC4:
				param.type = VEC4;
				break;
			default:
				offset += UnitConverter::PrimitiveSize(entry.type);
				continue;
			}
			offset += UnitConverter::PrimitiveSize(entry.type);
			param.primitive_size = UnitConverter::PrimitiveSize(entry.type);
			CreateParameter(param);
		}
		buffer_and_descriptor_table_sizes.insert(std::make_pair(index, offset));


	}
	catch (...) {
		throw std::runtime_error("Material Visible Constant buffer needs to be directly specified in the root signature with a valid layout attached.");
	}


}

int MaterialTemplate::GetMaterialTemplateParameterIndex(const std::string& name) const {
	auto fnd = material_parameters_map.find(name);
	if (fnd != material_parameters_map.end()) {
		return fnd->second;
	}
	else {
		throw std::runtime_error("Material template parameter " + name + " not found");
	}
}

void MaterialManager::UpdateMaterials()
{
	std::lock_guard<std::mutex> lock(material_load_mutex);
	for (auto& loaded_texture : material_load) {
		if (!loaded_texture.future.IsAvailable() || loaded_texture.destroyed) continue;
		try {
			auto texture_1 = loaded_texture.future.GetValue();
			loaded_texture.material->SetParameter(loaded_texture.name, texture_1);
			loaded_texture.destroyed = true;
		}
		catch (...) {
			loaded_texture.material->status = Material::Material_status::ERROR;
			loaded_texture.destroyed = true;
		}
	}


	while (!material_load.empty() && material_load.front().destroyed) {
		material_load.pop_front();
	}

}

void MaterialManager::AddTextureLoad(std::shared_ptr<Material> material, std::string name, Future<std::shared_ptr<RenderTexture2DResource>> future)
{
	std::lock_guard<std::mutex> lock(material_load_mutex);
	material_load.push_back(Material_loading_item{ name, material, future, false });
}

