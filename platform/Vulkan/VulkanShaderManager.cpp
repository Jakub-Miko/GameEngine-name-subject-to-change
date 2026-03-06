#include "VulkanShaderManager.h"
#include <Renderer/Renderer.h>
#include <platform/Vulkan/VulkanRenderCommandQueue.h>
#include "VulkanUnitConverter.h"
#include <fstream>
#include <FileManager.h>
#include <stdexcept>
#include "VulkanRenderContext.h"
#include <sstream>
#include <cstring>

VulkanShaderManager::VulkanShaderManager() {

}

VulkanParsedShader VulkanShaderManager::CompileShader(const std::string& name)
{
	auto file_name = FileManager::Get()->GetRenderApiAssetFilePath("/shaders/" + name);
	std::ifstream file_in(FileManager::Get()->GetRenderApiAssetFilePath(file_name));
	std::string source;
	if (file_in.is_open()) {
		std::stringstream stream;
		stream << file_in.rdbuf();
		source = stream.str();
		return std::move(LinkShader(ParseShader(source,file_name)));
	}
	else {
		throw std::runtime_error("Couldn't find the shader");
	}
}

VulkanParsedShader VulkanShaderManager::ParseShader(const std::string& source_code, const std::string& file_name)
{
	auto fnd_vertex = source_code.find("#Vertex");
	auto fnd_fragment = source_code.find("#Fragment");
	auto fnd_geometry = source_code.find("#Geometry");
	auto fnd_compute = source_code.find("#Compute");
	if (fnd_vertex != source_code.npos && fnd_fragment != source_code.npos) {
		auto end_vertex = source_code.find("#end\n", fnd_vertex) ;
		auto end_fragment = source_code.find("#end\n", fnd_fragment);
		fnd_vertex += strlen("#Vertex");
		fnd_fragment += strlen("#Fragment");
		VulkanParsedShader parsed;

		VulkanShaderSource vertex;
		vertex.type = VulkanShaderStages::VERTEX;
		vertex.source = source_code.substr(fnd_vertex, end_vertex - fnd_vertex);
		vertex.name = file_name;
		parsed.push_back(vertex);

		VulkanShaderSource fragment;
		fragment.type = VulkanShaderStages::FRAGMENT;
		fragment.source = source_code.substr(fnd_fragment, end_fragment - fnd_fragment);
		fragment.name = file_name;
		parsed.push_back(fragment);

		if (fnd_geometry != source_code.npos) {
			fnd_geometry += strlen("#Geometry");
			auto end_geometry = source_code.find("#end\n", fnd_geometry);
			VulkanShaderSource geometry;
			geometry.type = VulkanShaderStages::GEOMETRY;
			geometry.source = source_code.substr(fnd_geometry, end_geometry - fnd_geometry);
			geometry.name = file_name;
			parsed.push_back(geometry);
		}

		return parsed;

	}
	else if(fnd_compute != source_code.npos) {
		auto end_compute = source_code.find("#end\n", fnd_compute) ;
		fnd_compute += strlen("#Compute");
		VulkanParsedShader parsed;

		VulkanShaderSource compute;
		compute.type = VulkanShaderStages::COMPUTE;
		compute.source = source_code.substr(fnd_compute, end_compute - fnd_compute);
		compute.name = file_name;
		parsed.push_back(compute);

		return parsed;
	} else {
		throw std::runtime_error("Unsupported shader type.\n");
	}
}

VkShaderModule VulkanShaderManager::CompileShaderStage(VulkanShaderStages type, const std::string& source, const std::string& file_name , const std::vector<std::string>& compiler_definitions)
{
	DEFINE_VK_INSTANCE(context);
	
	VkShaderModule module;

	shaderc::CompileOptions options;

	options.SetTargetSpirv(shaderc_spirv_version_1_6);
	options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, shaderc_env_version::shaderc_env_version_vulkan_1_3);
	options.SetVulkanRulesRelaxed(true);
	options.SetAutoBindUniforms(true);
	options.SetAutoMapLocations(true);
	options.SetSourceLanguage(shaderc_source_language_glsl);
	options.SetIncluder(std::make_unique<VulkanShaderIncluder>());

	for(auto& def: compiler_definitions) {
		auto equal_sign = def.find('=');
		if(equal_sign != std::string::npos) {
			std::string key = def.substr(0, equal_sign);
			std::string value = def.substr(equal_sign + 1);
			options.AddMacroDefinition(key.c_str(), key.size(), value.c_str(), value.size());
		} else {
			options.AddMacroDefinition(def.c_str(), def.size(), nullptr, 0);
		}
	}
#ifndef NDEBUG
	options.SetGenerateDebugInfo();
	options.SetOptimizationLevel(shaderc_optimization_level_zero);

#endif
	shaderc::Compiler compiler = shaderc::Compiler();
	auto result = compiler.CompileGlslToSpv(source, VulkanUnitConverter::ShaderStageToShadercShaderKind(type), file_name.c_str(), "main", options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		auto message = result.GetErrorMessage();
		throw std::runtime_error(message);
	}

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pCode = result.begin();
	info.codeSize = (result.end() - result.begin()) * sizeof(uint32_t);
	
	vkCreateShaderModule(context->GetVkDevice(), &info, NULL, &module);

	return module;
}

VulkanParsedShader VulkanShaderManager::LinkShader(VulkanParsedShader shader, const std::vector<std::string>& compiler_definitions)
{

	for (auto& shader_stage : shader) {
		VkShaderModule shader_stage_compiled = CompileShaderStage(shader_stage.type, shader_stage.source, shader_stage.name, compiler_definitions);
		shader_stage.module = shader_stage_compiled;
	}

	return std::move(shader);
}

shaderc_include_result* VulkanShaderIncluder::GetInclude(const char* requested_source, shaderc_include_type type,
	const char* requesting_source, size_t include_depth) {
	auto* result = new shaderc_include_result;

	std::string file_path;
	if(type == shaderc_include_type_relative && strlen(requesting_source) > 0) {
		auto fnd = std::string(requesting_source).find_last_of('/');
		if(fnd != std::string::npos) {
			auto parent_path = std::string(requesting_source).substr(0, fnd);
			file_path = FileManager::Get()->GetPath(std::string(parent_path) + "/" + std::string(requested_source));
		} else {
			file_path = FileManager::Get()->GetRenderApiAssetFilePath(std::string(requested_source));
		}
	} else {
		file_path = FileManager::Get()->GetRenderApiAssetFilePath(std::string(requested_source));
	}

	std::ifstream file_in(file_path);
	if (!file_in.is_open()) {
		auto* data = new ResultUserData;
		data->source_name = "";
		data->content = "Couldn't find the include file: " + std::string(requested_source);
		result->content = data->content.c_str();
		result->content_length = data->content.size();
		result->source_name = "";
		result->source_name_length = 0;
		result->user_data = data;
		return result;
	}

	std::stringstream stream;
	stream << file_in.rdbuf();
	auto* data = new ResultUserData;
	data->source_name = file_path;
	data->content = std::move(stream.str());
	result->content = data->content.c_str();
	result->content_length = data->content.size();
	result->source_name = data->source_name.c_str();
	result->source_name_length = data->source_name.size();
	result->user_data = data;
	return result;
}

void VulkanShaderIncluder::ReleaseInclude(shaderc_include_result* data) {
	if(data->user_data) {
		delete static_cast<ResultUserData*>(data->user_data);
	}
	delete data;
}

VulkanShaderManager::~VulkanShaderManager() {
	for (auto shader : m_Shaders) {
		delete shader.second;
	}
}

Shader* VulkanShaderManager::GetShader_impl(const std::string& name) {
	std::unique_lock<std::mutex> lock(sync_mutex);
	auto fnd = m_Shaders.find(name);
	if (fnd != m_Shaders.end()) {
		return static_cast<Shader*>(fnd->second);
	}
	else {
		VulkanShader* new_shader = (m_Shaders.insert(std::make_pair(name, static_cast<VulkanShader*>(CreateShader_impl(name)))).first->second);
		lock.unlock();
		return static_cast<Shader*>(new_shader);
	}
}

Shader* VulkanShaderManager::CreateShader_impl(const std::string& path)
{
	VulkanShader* new_shader = new VulkanShader;
	VulkanParsedShader shader_stages = CompileShader(path);
	for (auto& shader_stage : shader_stages) {
		new_shader->SetStage(shader_stage.module, shader_stage.type);
	}
	new_shader->SetId(path);

	return static_cast<Shader*>(new_shader);
}

Shader* VulkanShaderManager::CreateShaderFromString_impl(const std::string& source, const std::vector<std::string>& compiler_definitions, const std::string& file_name )
{
	VulkanShader* new_shader = new VulkanShader;
	VulkanParsedShader shader_stages = LinkShader(ParseShader(source, file_name), compiler_definitions);
	for (auto& shader_stage : shader_stages) {
		new_shader->SetStage(shader_stage.module, shader_stage.type);
	}
	new_shader->SetId("Unknown");

	return static_cast<Shader*>(new_shader);
}

void VulkanShader::ResetStage(VulkanShaderStages stage_type)
{
	DEFINE_VK_INSTANCE(context);
	auto& stage = shader_modules[(int)stage_type];
	if (stage.defined) {
		vkDestroyShaderModule(context->GetVkDevice(), stage.stage, NULL);
	}
	stage.defined = false;
}

void VulkanShader::SetStage(VkShaderModule stage, VulkanShaderStages stage_type)
{
	ResetStage(stage_type);
	shader_modules[(int)stage_type].stage = stage;
	shader_modules[(int)stage_type].defined = true;
}

VkShaderModule* VulkanShader::GetStage(VulkanShaderStages stage_type)
{
	return shader_modules[(int)stage_type].defined ? &shader_modules[(int)stage_type].stage : nullptr;
}

VulkanShader::~VulkanShader()
{
	for (int i = 0; i < (int)VulkanShaderStages::NUM_OF_STAGES; i++) {
		ResetStage((VulkanShaderStages)i);
	}
}
