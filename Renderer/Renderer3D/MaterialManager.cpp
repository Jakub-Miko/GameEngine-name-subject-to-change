#include "MaterialManager.h"
#include <ConfigManager.h>
#include <fstream>
#include <json.hpp>
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

#ifdef Vulkan_API
#include <platform/Vulkan/VulkanRenderDescriptorHeap.h>
#endif


NLOHMANN_JSON_SERIALIZE_ENUM(MaterialLayoutItemType, {
	{MaterialLayoutItemType::INT , "int"},
	{MaterialLayoutItemType::INT , "INT"},
	{MaterialLayoutItemType::MAT3 , "mat3"},
	{MaterialLayoutItemType::MAT3 , "MAT3"},
	{MaterialLayoutItemType::MAT4 , "mat4"},
	{MaterialLayoutItemType::MAT4 , "MAT4"},
	{MaterialLayoutItemType::VEC2 , "vec2"},
	{MaterialLayoutItemType::VEC3 , "vec3"},
	{MaterialLayoutItemType::VEC4 , "vec4"},
	{MaterialLayoutItemType::VEC2 , "VEC2"},
	{MaterialLayoutItemType::VEC3 , "VEC3"},
	{MaterialLayoutItemType::VEC4 , "VEC4"},
	{MaterialLayoutItemType::SCALAR , "scalar"},
	{MaterialLayoutItemType::SCALAR , "float"},
	{MaterialLayoutItemType::SCALAR , "FLOAT"},
	{MaterialLayoutItemType::SCALAR , "SCALAR"},
	{MaterialLayoutItemType::TEXTURE , "texture"},
	{MaterialLayoutItemType::TEXTURE , "texture_2d"},
	{MaterialLayoutItemType::TEXTURE , "texture_2D"},
	{MaterialLayoutItemType::TEXTURE , "TEXTURE"},
	{MaterialLayoutItemType::TEXTURE_2D_ARRAY , "texture_array"},
	{MaterialLayoutItemType::TEXTURE_2D_ARRAY , "texture_2d_array"},
	{MaterialLayoutItemType::TEXTURE_2D_ARRAY , "texture_2D_array"},
	{MaterialLayoutItemType::TEXTURE_2D_CUBEMAP , "texture_cubemap"},
	{MaterialLayoutItemType::TEXTURE_2D_CUBEMAP , "texture_2d_cubemap"},
	{MaterialLayoutItemType::TEXTURE_2D_CUBEMAP , "texture_2D_cubemap"},
	{MaterialLayoutItemType::CONSTANT_BUFFER , "constant_buffer"}
	})

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
	try {
		auto material = ParseMaterialFromFile(path);
		material->material_path = path_in;
		materials.insert(std::make_pair(path, material));
		return material;

	}
	catch(...) {
		auto mat = std::make_shared<Material>();
		mat->status = Material::Material_status::ERROR;
		return std::make_shared<Material>();
	}
}

MaterialManager::MaterialManager() : materials(), material_templates(), material_mutex(), material_load(), material_load_mutex()
{
	if (ConfigManager::Get()->Exists("PreloadMaterialTemplates")) {
		auto preload = ConfigManager::Get()->GetArray("PreloadMaterialTemplates");
		for (int i = 0; i < preload->GetArraySize(); i++) {
			auto item = preload->GetString(i);
			LoadMaterialTemplateFile(item);
		}
	}
}

void MaterialManager::ClearMaterialCache()
{
	std::lock_guard<std::mutex> lock1(material_load_mutex);
	std::lock_guard<std::mutex> lock2(material_mutex);
	// material_templates.clear();
	materials.clear();
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
	return mat;
}

std::shared_ptr<Material> MaterialManager::ParseMaterialFromString(const std::string& string, std::shared_ptr<MaterialTemplate> material_template)
{
	using namespace nlohmann;
	json material_json = json::parse(string);
	if (material_template == nullptr) {
		if (!material_json.contains("material_template")) throw std::runtime_error("Material format undefined, must provide a material_template name");
		std::string material_template_name = material_json["material_template"].get<std::string>();
		auto fnd_template = material_templates.find(material_template_name);
		if (fnd_template != material_templates.end()) {
			material_template = fnd_template->second;
		}
		else {
			throw std::runtime_error("Material format undefined, the provided material_template is invalid, make sure to load the material template before using its materials");
		}
	}

	std::shared_ptr<Material> material = std::make_shared<Material>(material_template);
	if (!material_json.contains("parameters") || (!material_json["parameters"].is_array() && !material_json["parameters"].is_null())) throw std::runtime_error("Material file must contain a parameters list");
	if (!material_json["parameters"].is_null()) {
		for (auto& parameter : material_json["parameters"]) {
			auto type = parameter["type"].get<MaterialLayoutItemType>();
			switch (type)
			{
			case MaterialLayoutItemType::INT:
				material->SetParameter<int>(parameter["name"].get<std::string>(), parameter["value"].get<int>());
				break;
			case MaterialLayoutItemType::MAT3:
				material->SetParameter<glm::mat3>(parameter["name"].get<std::string>(), parameter["value"].get<glm::mat3>());
				break;
			case MaterialLayoutItemType::MAT4:
				material->SetParameter<glm::mat4>(parameter["name"].get<std::string>(), parameter["value"].get<glm::mat4>());
				break;
			case MaterialLayoutItemType::SCALAR:
				material->SetParameter<float>(parameter["name"].get<std::string>(), parameter["value"].get<float>());
				break;
			case MaterialLayoutItemType::VEC2:
				material->SetParameter<glm::vec2>(parameter["name"].get<std::string>(), parameter["value"].get<glm::vec2>());
				break;
			case MaterialLayoutItemType::VEC3:
				material->SetParameter<glm::vec3>(parameter["name"].get<std::string>(), parameter["value"].get<glm::vec3>());
				break;
			case MaterialLayoutItemType::VEC4:
				material->SetParameter<glm::vec4>(parameter["name"].get<std::string>(), parameter["value"].get<glm::vec4>());
				break;
			case MaterialLayoutItemType::TEXTURE:
				if (parameter["value"].get<std::string>().empty()) {
					if (parameter.contains("default_normal") && parameter["default_normal"].get<bool>()) {
						material->SetParameter(parameter["name"].get<std::string>(), TextureManager::Get()->GetDefaultNormalTexture());
					}
					else {
						material->SetParameter(parameter["name"].get<std::string>(), TextureManager::Get()->GetDefaultTexture());
					}
				}
				else {
					material->SetTexture(parameter["name"].get<std::string>(), parameter["value"].get<std::string>());
				}
				break;
			default:
				throw std::runtime_error("Invalid material type.\n");
			}
		}
	}
	material->material_path = "";
	// material->UpdateValues(); Update will happen on first use
	return material;
}

void MaterialManager::SerializeMaterial(const std::string& filepath, std::shared_ptr<Material> material)
{
	using namespace nlohmann;
	std::ofstream file(FileManager::Get()->GetPath(filepath));
	if (!file.is_open()) throw std::runtime_error("File " + FileManager::Get()->GetPath(filepath) + " could not be opened");
	
	
	json json_object;
	json& parameters = json_object["parameters"];
	for (auto& param : material->parameters) {
		std::string texture;
		if (bool(param.flags & Material::MaterialParameter_flags::DEFAULT)) continue;
		json parameter_json = json::object();
		parameter_json["name"] = param.name;
		switch (param.type)
		{
		case MaterialLayoutItemType::INT:
			parameter_json["type"] = "INT";
			parameter_json["value"] = std::get<int>(param.resource);
			break;
		case MaterialLayoutItemType::SCALAR:
			parameter_json["type"] = "SCALAR";
			parameter_json["value"] = std::get<float>(param.resource);
			break;
		case MaterialLayoutItemType::VEC2:
			parameter_json["type"] = "VEC2";
			parameter_json["value"] = std::get<glm::vec2>(param.resource);
			break;
		case MaterialLayoutItemType::VEC3:
			parameter_json["type"] = "VEC3";
			parameter_json["value"] = std::get<glm::vec3>(param.resource);
			break;
		case MaterialLayoutItemType::VEC4:
			parameter_json["type"] = "VEC4";
			parameter_json["value"] = std::get<glm::vec4>(param.resource);
			break;
		case MaterialLayoutItemType::TEXTURE:
			parameter_json["type"] = "TEXTURE";
			texture = std::get<MaterialTextureType>(param.resource).path;
			if (texture.empty()) throw std::runtime_error("Filepath to a texture could not be found during material serialization.");
			parameter_json["value"] = texture;
			break;
		default:
			throw std::runtime_error("Invalid type during material serialization");
		}
		parameters.push_back(parameter_json);
	}
	json_object["material_template"] = material->GetMaterialTemplate()->GetName();
	std::string json_dump = json_object.dump();

	file << json_dump;

	file.close();
}

std::shared_ptr<Material> MaterialManager::CreateEmptyMaterial(const std::string& filepath_in, std::shared_ptr<MaterialTemplate> material_template)
{
	std::string file_path = FileManager::Get()->GetPath(filepath_in);
	auto material = std::make_shared<Material>(material_template);
	material->material_path = filepath_in;
	SerializeMaterial(filepath_in, material);
	std::lock_guard<std::mutex> lock(material_mutex);
	materials.insert(std::make_pair(file_path, material));
	return material;
}

void Material::SetMaterial(RenderCommandList* command_list)
{
	/*UpdateValues(command_list);
	auto& sig = material_template->GetRootSignature().GetDescriptor().parameters;
	for (auto& resource : resources) {
		if (resource.is_table) {
			command_list->SetDescriptorTable(sig[resource.index].name, std::get<FrameMultiBufferResource<RenderDescriptorTable>>(resource.resource).GetResource());
		}
		else {
			command_list->SetConstantBuffer(sig[resource.index].name, std::get<std::shared_ptr<RenderBufferResource>>(resource.resource));
		}
	}
	for (auto& parameter : parameters) {
		if (parameter.type == MaterialTemplate::MaterialTemplateParameterType::TEXTURE && parameter.IsDirty()) {
			command_list->SetTexture2D(parameter.name, std::get<Texture_type>(parameter.resource).texture);
		} else if (parameter.type == MaterialTemplate::MaterialTemplateParameterType::TEXTURE_2D_ARRAY && parameter.IsDirty()) {
			command_list->SetTexture2DArray(parameter.name, std::get<std::shared_ptr<RenderTexture2DArrayResource>>(parameter.resource));
		} else if (parameter.type == MaterialTemplate::MaterialTemplateParameterType::TEXTURE_2D_CUBEMAP && parameter.IsDirty()) {
			command_list->SetTexture2DCubemap(parameter.name, std::get<std::shared_ptr<RenderTexture2DCubemapResource>>(parameter.resource));
		}
	}*/
	command_list->SetMaterial(GetMaterialTemplate()->GetName(), shared_from_this());
}


//void Material::UpdateValues(RenderCommandList* command_list)
//{
//	for (auto& parameter : parameters) {
//		auto& param_info = material_template->GetMaterialTemplateParameter(parameter.name);
//		if (param_info.descriptor_table_id != -1) {
//			if (parameter.type == MaterialTemplate::TEXTURE) {
//				if (param_info.descriptor_table_id != -1) {
//					auto res_fnd = std::find_if(resources.begin(), resources.end(), [&param_info](const MaterialResource& res) {return res.index == param_info.descriptor_table_id; });
//					if (res_fnd != resources.end()) {
//						RenderResourceManager::Get()->CreateTexture2DDescriptor(std::get<FrameMultiBufferResource<RenderDescriptorTable>>(res_fnd->resource).GetResource(),
//							param_info.index, std::get<Texture_type>(parameter.resource).texture);
//						parameter.flags &= ~MaterialParameter_flags::DIRTY;
//					}
//					else {
//						throw std::runtime_error("Descriptor table for parameter " + parameter.name + " not found");
//					}
//				}
//			}
//			else if (parameter.type == MaterialTemplate::TEXTURE_2D_ARRAY) {
//				if (param_info.descriptor_table_id != -1) {
//					auto res_fnd = std::find_if(resources.begin(), resources.end(), [&param_info](const MaterialResource& res) {return res.index == param_info.descriptor_table_id; });
//					if (res_fnd != resources.end()) {
//						RenderResourceManager::Get()->CreateTexture2DArrayDescriptor(std::get<FrameMultiBufferResource<RenderDescriptorTable>>(res_fnd->resource).GetResource(),
//							param_info.index, std::get<std::shared_ptr<RenderTexture2DArrayResource>>(parameter.resource));
//						parameter.flags &= ~MaterialParameter_flags::DIRTY;
//					}
//					else {
//						throw std::runtime_error("Descriptor table for parameter " + parameter.name + " not found");
//					}
//				}
//			}
//			else if (parameter.type == MaterialTemplate::TEXTURE_2D_CUBEMAP) {
//				if (param_info.descriptor_table_id != -1) {
//					auto res_fnd = std::find_if(resources.begin(), resources.end(), [&param_info](const MaterialResource& res) {return res.index == param_info.descriptor_table_id; });
//					if (res_fnd != resources.end()) {
//						RenderResourceManager::Get()->CreateTexture2DCubemapDescriptor(std::get<FrameMultiBufferResource<RenderDescriptorTable>>(res_fnd->resource).GetResource(),
//							param_info.index, std::get<std::shared_ptr<RenderTexture2DCubemapResource>>(parameter.resource));
//						parameter.flags &= ~MaterialParameter_flags::DIRTY;
//					}
//					else {
//						throw std::runtime_error("Descriptor table for parameter " + parameter.name + " not found");
//					}
//				}
//			} else if(parameter.IsDirty()) {
//				auto res_fnd = std::find_if(resources.begin(), resources.end(), [&param_info](const MaterialResource& res) {return res.index == param_info.index; });
//				if(res_fnd != resources.end()) {
//					RenderResourceManager::Get()->UploadDataToBuffer(command_list, std::get<std::shared_ptr<RenderBufferResource>>(res_fnd->resource),
//						&parameter.resource, param_info.primitive_size, param_info.constant_buffer_offset);
//					parameter.flags &= ~MaterialParameter_flags::DIRTY;
//				}
//				else {
//					throw std::runtime_error("Constant Buffer for parameter " + parameter.name + " not found");
//				}
//			}
//		}
//	}
//}

// Be aware this is not thread safe
void Material::UpdateValues(RenderCommandList* command_list)
{
	command_list->UpdateMaterial(shared_from_this());
}

void Material::SetTexture(const std::string& name, const std::string& path)
{
	if (TextureManager::Get()->IsTextureAvailable(path)) {
		SetParameter(name, TextureManager::Get()->LoadTextureFromFile(path, false), path);
	}
	else {
		auto future = TextureManager::Get()->LoadTextureFromFileAsync(path, true);
		MaterialManager::Get()->AddTextureLoad(shared_from_this(),name, future, path);
	}
}
//
//Material::Material(std::shared_ptr<MaterialTemplate> material_template) : material_template(material_template), parameters(), resources()
//{
//	bool has_defaults = material_template->GetShader()->GetDefaultMaterial() != nullptr;
//	std::shared_ptr<Material> defaults = nullptr;
//	if (has_defaults) {
//		defaults = material_template->GetShader()->GetDefaultMaterial();
//		for (auto& parameter : defaults->parameters) {
//			MaterialParameter res;
//			res.flags = parameter.flags | MaterialParameter_flags::DEFAULT;
//			res.name = parameter.name;
//			res.type = parameter.type;
//			res.resource = parameter.resource;
//			res.flags |= MaterialParameter_flags::DIRTY;
//			parameters.push_back(res);
//		}
//	}
//	else {
//		for (auto& material_param : material_template->GetMaterialTemplateParameters()) {
//			MaterialParameter res;
//			res.flags |= MaterialParameter_flags::DIRTY | MaterialParameter_flags::DEFAULT;
//			res.name = material_param.name;
//			res.type = material_param.type;
//			res.flags |= material_param.descriptor_table_id != -1 ? MaterialParameter_flags::TABLE : (MaterialParameter_flags)0;
//			SetParameterTypeDefault(res);
//			parameters.push_back(res);
//		}
//	}
//
//
//
//	std::set<int> const_buf_index;
//	std::set<int> table_index;
//	const auto& signature = material_template->GetRootSignature();
//	for (auto& param_types : material_template->GetMaterialTemplateParameters()) {
//		if (param_types.constant_buffer_offset != -1) {
//			if (const_buf_index.insert(param_types.index).second) {
//				int size = material_template->GetTableOrBufferSize(param_types.index);
//				MaterialResource res;
//				res.index = param_types.index;
//				res.is_table = false;
//				RenderBufferDescriptor desc(size, RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
//				res.resource = RenderResourceManager::Get()->CreateBuffer(desc);
//				resources.push_back(res);
//			}
//		}
//		else if (param_types.descriptor_table_id != -1) {
//			if (table_index.insert(param_types.descriptor_table_id).second) {
//				int size = material_template->GetTableOrBufferSize(param_types.descriptor_table_id);
//				MaterialResource res;
//				res.index = param_types.descriptor_table_id;
//				res.is_table = true;
//				res.resource = FrameMultiBufferResource<RenderDescriptorTable>([size]() {return Renderer3D::Get()->GetDescriptorHeap().Allocate(size); });
//				resources.push_back(res);
//			}
//		}
//	}
//	status = Material_status::OK;
//}

void Material::SetParameterTypeDefault(MaterialParameter& param)
{
	switch (param.type)
	{
	case MaterialLayoutItemType::SCALAR:
		param.resource = 1.0f;
		break;
	case MaterialLayoutItemType::VEC2:
		param.resource = glm::vec2(1.0f);
		break;
	case MaterialLayoutItemType::VEC3:
		param.resource = glm::vec3(1.0f);
		break;
	case MaterialLayoutItemType::VEC4:
		param.resource = glm::vec4(1.0f);
		break;
	case MaterialLayoutItemType::INT:
		param.resource = 0;
		break;
	case MaterialLayoutItemType::TEXTURE:
		param.resource = MaterialTextureType{ TextureManager::Get()->GetDefaultTexture() };
		break;
	case MaterialLayoutItemType::TEXTURE_2D_ARRAY:
		param.resource = TextureManager::Get()->GetDefaultTextureArray();
		break;
	case MaterialLayoutItemType::TEXTURE_2D_CUBEMAP:
		param.resource = TextureManager::Get()->GetDefaultTextureCubemap();
		break;
	default:
		throw std::runtime_error("Unsupported tempate parameter type " + param.name);
		break;
	}
}

//MaterialTemplate::MaterialTemplate(std::shared_ptr<Shader> shader_in) : material_parameters(), material_parameters_map(), shader_wk(shader_in), buffer_and_descriptor_table_sizes()
//{
//	auto shader = GetShader();
//	auto& desc_parametrs = shader->GetRootSignature().GetDescriptor().parameters;
//	int index = 0;
//	for (auto& parameter : desc_parametrs) {
//		if (parameter.is_material_visible) {
//			switch (parameter.type)
//			{
//			case RootParameterType::TEXTURE_2D:
//				AddTexture2DParameter(parameter,index);
//				break;
//			case RootParameterType::TEXTURE_2D_ARRAY:
//				AddTexture2DArrayParameter(parameter, index);
//				break;
//			case RootParameterType::TEXTURE_2D_CUBEMAP:
//				AddTexture2DCubemapParameter(parameter, index);
//				break;
//			case RootParameterType::CONSTANT_BUFFER:
//				AddConstantBufferParameter(parameter, index);
//				break;
//			case RootParameterType::DESCRIPTOR_TABLE:
//				AddDescriptorTableParameter(parameter, index);
//				break;
//			default:
//				break;
//			}
//		}
//		index++;
//	}
//}

const MaterialLayoutItem& MaterialTemplate::GetMaterialTemplateParameter(const std::string& name) const
{
	auto fnd = material_parameters_map.find(name);
	if (fnd != material_parameters_map.end()) {
		return material_parameters.layout_items[fnd->second];
	}
	else {
		throw std::runtime_error("Material template parameter " + name + " not found");
	}
}

MaterialTemplate::~MaterialTemplate()
{

}

//
//int MaterialTemplate::GetTableOrBufferSize(int index)
//{
//	auto fnd = buffer_and_descriptor_table_sizes.find(index);
//	if (fnd != buffer_and_descriptor_table_sizes.end()) {
//		return fnd->second;
//	}
//	else {
//		throw std::runtime_error("Table or buffer size " + std::to_string(index) + " not found");
//	}
//	
//}

//void MaterialTemplate::CreateParameter(const MaterialTemplateParameter& parameter)
//{
//	material_parameters.push_back(parameter);
//	material_parameters_map.insert(std::make_pair(parameter.name,material_parameters.size()-1));
//}


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
#ifdef EDITOR
			loaded_texture.material->SetParameter(loaded_texture.name, texture_1, loaded_texture.path);
#else
			loaded_texture.material->SetParameter(loaded_texture.name, texture_1);
#endif
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

void MaterialManager::AddTextureLoad(std::shared_ptr<Material> material, std::string name, Future<std::shared_ptr<RenderTexture2DResource>> future, const std::string path)
{
	std::lock_guard<std::mutex> lock(material_load_mutex);
#ifdef EDITOR
	material_load.push_back(Material_loading_item{ name, material, future, false, path });
#else
	material_load.push_back(Material_loading_item{ name, material, future, false });
#endif
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(const std::string& material_template_name) {
	std::shared_ptr<MaterialTemplate> mat_template;
	auto fnd_template = material_templates.find(material_template_name);
	if (fnd_template != material_templates.end()) {
		mat_template = fnd_template->second;
	}
	else {
		throw std::runtime_error("Material could not be created since the material template " + material_template_name + " was not loaded.\n");
	}
	
	return std::make_shared<Material>(mat_template);

}

bool Material::MaterialParameter::IsDirty() const {
	return (bool)(flags & MaterialParameter_flags::DIRTY);
}


void Material::ActivateParameter(const std::string& name) {
	auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
	param.flags &= ~MaterialParameter_flags::DEFAULT;
}

void Material::DeactivateParameter(const std::string& name) {
	auto& param = parameters[GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name)];
	param.flags |= MaterialParameter_flags::DEFAULT | MaterialParameter_flags::DIRTY;
	auto def = GetMaterialTemplate()->GetDefaultMaterial();
	if (def != nullptr) {
		auto param_src = GetMaterialTemplate()->GetMaterialTemplateParameterIndex(name);
		param.resource = def->parameters[param_src].resource;
	}
	else {
		SetParameterTypeDefault(param);
	}

}

std::shared_ptr<MaterialTemplate> MaterialManager::LoadMaterialTemplateFromJson(const nlohmann::json& json_object, const std::string& name_in)
{
	if (!json_object.is_array()) {
		throw std::runtime_error("Material template must be a named list of parameters.\n");
	}

	auto name = name_in.empty() ? std::string(json_object.type_name()) : name_in;

	MaterialLayout layout;
	for (auto& item : json_object) {
		if (!item.is_object()) {
			throw std::runtime_error("Material template item must be a json object.\n");
		}
		if (!item.contains("name") || !item["name"].is_string() || !item.contains("type") || !item["type"].is_string()) {
			throw std::runtime_error("Material template item have a string \"name\" and \"type\".\n");
		}
		MaterialLayoutItem mat_item;
		mat_item.name = item["name"].get<std::string>();
		mat_item.type = item["type"].get<MaterialLayoutItemType>();

		bool default_exists = item.contains("default_value");

		
		switch (mat_item.type)
		{
		case MaterialLayoutItemType::INT:
		mat_item.default_value = default_exists ? item["default_value"].get<int>() : 0;
		break;
		case MaterialLayoutItemType::MAT3:
		mat_item.default_value = default_exists ? item["default_value"].get<glm::mat3>() : glm::mat3(1.0f);
		break;
		case MaterialLayoutItemType::MAT4:
		mat_item.default_value = default_exists ? item["default_value"].get<glm::mat4>() : glm::mat4(1.0f);
		break;
		case MaterialLayoutItemType::SCALAR:
		mat_item.default_value = default_exists ? item["default_value"].get<float>() : 0.0f;
		break;
		case MaterialLayoutItemType::VEC2:
		mat_item.default_value = default_exists ? item["default_value"].get<glm::vec2>() : glm::vec2(0,0);
		break;
		case MaterialLayoutItemType::VEC3:
		mat_item.default_value = default_exists ? item["default_value"].get<glm::vec3>() : glm::vec3(0, 0, 0);
		break;
		case MaterialLayoutItemType::VEC4:
		mat_item.default_value = default_exists ? item["default_value"].get<glm::vec4>() : glm::vec4(0, 0, 0, 0);
		break;
		case MaterialLayoutItemType::TEXTURE:{
			bool is_normal_texture = item.contains("normal_texture") && item["normal_texture"].get<bool>();
			if(default_exists) {
				mat_item.default_value = item["default_value"].get<std::string>();
			} else if (is_normal_texture) {
				mat_item.default_value = MaterialTextureType { TextureManager::Get()->GetDefaultNormalTexture(), ""};
			} else {
				mat_item.default_value = MaterialTextureType { TextureManager::Get()->GetDefaultTexture(), ""};
			}
		}
		break;
		case MaterialLayoutItemType::TEXTURE_2D_ARRAY:		// Texture arrays, cubemaps, and constant buffers, cannot be have defaults specified
		case MaterialLayoutItemType::TEXTURE_2D_CUBEMAP:	
		case MaterialLayoutItemType::CONSTANT_BUFFER:		
			mat_item.default_value = std::monostate();
			break;
		default:
			throw std::runtime_error("Invalid material type.\n");
		}
		layout.layout_items.push_back(mat_item);
	}


	return MaterialTemplate::CreateTemplate(layout, name);
}

MaterialTemplate::MaterialTemplate(const MaterialLayout& layout, std::string name, Private dummy) : material_name(name), material_parameters(layout),  material_parameters_map(), default_material(nullptr)
{
#ifdef Vulkan_API
	material_allocator = std::make_unique<VulkanRenderDescriptorHeap>(material_parameters);
#elif
	static_assert(false, "Apis other that vulkan are currently unsupported");
#endif
	for (int i = 0; i < material_parameters.layout_items.size(); i++) {
		material_parameters_map.insert(std::make_pair(material_parameters.layout_items[i].name, i));
	}


}

void MaterialManager::LoadMaterialTemplateFile(const std::string& path_in)
{
	auto path = FileManager::Get()->GetPath(path_in);
	auto fnd = loaded_signature_files.find(path);
	if (fnd != loaded_signature_files.end()) {
		return;
	}
	loaded_signature_files.insert(path);

	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Material template file could not be opened.");
	}
	nlohmann::json json_object;

	json_object << file;

	file.close();

	if (!json_object.is_object()) {
		throw std::runtime_error("Material template file must contain an object containing json objects representing the materials");
	}

	for (auto& element : json_object.items()) {
		auto material_template = LoadMaterialTemplateFromJson(element.value(), element.key());
		RegisterMaterialTemplate(material_template);
	}
}

void MaterialManager::RegisterMaterialTemplate(std::shared_ptr<MaterialTemplate> material_template)
{
	auto fnd = material_templates.find(material_template->GetName());
	if (fnd != material_templates.end()) {
		throw std::runtime_error("Material template " + material_template->GetName() + " could not be loaded because it already exists,  \
			make sure each template has a unique name and is defined only once.\n");
	}
	
	material_templates.insert(std::make_pair(material_template->GetName(), material_template));
}

RenderDescriptorAllocationHandle MaterialTemplate::AllocateMaterialDescriptor()
{
	return material_allocator->Allocate();
}

Material::Material(std::shared_ptr<MaterialTemplate> material_template) : material_template(material_template), descriptor_table()
{
	auto default_temp = material_template->GetDefaultMaterial();
	descriptor_table = !default_temp ? nullptr : default_temp->descriptor_table; // Use the default values, first and on first used of set material or update create the actual table
	auto const_size = material_template->GetMaterialTemplateParameters().const_buffer_size;
	if (const_size != 0) {
		RenderBufferDescriptor desc;
		desc.buffer_size = const_size;
		desc.type = RenderBufferType::DEFAULT;
		desc.usage = RenderBufferUsage::CONSTANT_BUFFER;
		constant_buffer = RenderResourceManager::Get()->CreateBuffer(desc);
	}

	for (int i = 0; i < material_template->GetMaterialTemplateParameters().layout_items.size(); i++) {
		auto& layout_item = material_template->GetMaterialTemplateParameters().layout_items[i];
		MaterialParameter param;
		param.flags |= MaterialParameter_flags::DEFAULT;
		param.flags |= std::holds_alternative<std::monostate>(layout_item.default_value) ? (MaterialParameter_flags)0 : MaterialParameter_flags::DIRTY;
		param.name = layout_item.name;
		param.resource = layout_item.default_value;
		param.type = layout_item.type;
		parameters.push_back(param);
	}

	status = Material_status::UNINITIALIZED;
}

std::shared_ptr<MaterialTemplate> MaterialManager::GetMaterialTemplate(const std::string& name)
{
	auto fnd_template = material_templates.find(name);
	if (fnd_template != material_templates.end()) {
		return fnd_template->second;
	}
	else {
		throw std::runtime_error("Material template " + name + " was not loaded.\n");
	}
}

std::shared_ptr<MaterialTemplate> MaterialTemplate::CreateTemplate(const MaterialLayout& layout, std::string name)
{
	auto temp = std::make_shared<MaterialTemplate>(layout, name, Private());

	temp->default_material.reset();

	temp->default_material = std::make_shared<Material>(temp->shared_from_this());

	auto list = Renderer::Get()->GetRenderCommandList();

	temp->default_material->UpdateValues(list); // Make sure to initialize the actual material

	Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);

	return temp;
}
