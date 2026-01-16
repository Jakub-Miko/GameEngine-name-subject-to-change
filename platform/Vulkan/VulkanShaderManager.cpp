#include "VulkanShaderManager.h"
#include <Renderer/Renderer.h>
#include <platform/Vulkan/VulkanRenderCommandQueue.h>
#include "VulkanUnitConverter.h"
#include <fstream>
#include <FileManager.h>
#include <stdexcept>
#include "VulkanRenderContext.h"
#include "shaderc/shaderc.hpp"
#include <sstream>
#include <cstring>

VulkanShaderManager::VulkanShaderManager() {

}

VulkanParsedShader VulkanShaderManager::CompileShader(const std::string& name)
{
	std::ifstream file_in(FileManager::Get()->GetRenderApiAssetFilePath("/shaders/" + name));
	std::string source;
	if (file_in.is_open()) {
		std::stringstream stream;
		stream << file_in.rdbuf();
		source = stream.str();
		return std::move(LinkShader(ParseShader(source)));
	}
	else {
		throw std::runtime_error("Couldn't find the shader");
	}
}

VulkanParsedShader VulkanShaderManager::ParseShader(const std::string& source_code)
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
		parsed.push_back(vertex);

		VulkanShaderSource fragment;
		fragment.type = VulkanShaderStages::FRAGMENT;
		fragment.source = source_code.substr(fnd_fragment, end_fragment - fnd_fragment);
		parsed.push_back(fragment);

		if (fnd_geometry != source_code.npos) {
			fnd_geometry += strlen("#Geometry");
			auto end_geometry = source_code.find("#end\n", fnd_geometry);
			VulkanShaderSource geometry;
			geometry.type = VulkanShaderStages::GEOMETRY;
			geometry.source = source_code.substr(fnd_geometry, end_geometry - fnd_geometry);
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
		parsed.push_back(compute);

		return parsed;
	} else {
		throw std::runtime_error("Unsupported shader type.\n");
	}
}

VkShaderModule VulkanShaderManager::CompileShaderStage(VulkanShaderStages type, const std::string& source, const std::vector<std::string>& compiler_definitions)
{
	DEFINE_VK_INSTANCE(context);
	
	VkShaderModule module;
	
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_6);
	shaderc_compile_options_set_target_env(options, shaderc_target_env::shaderc_target_env_vulkan, shaderc_env_version::shaderc_env_version_vulkan_1_3);
	shaderc_compile_options_set_vulkan_rules_relaxed(options, true);
	shaderc_compile_options_set_auto_bind_uniforms(options, true);
	shaderc_compile_options_set_auto_map_locations(options, true);
	for(auto& def: compiler_definitions) {
		auto equal_sign = def.find('=');
		if(equal_sign != std::string::npos) {
			std::string key = def.substr(0, equal_sign);
			std::string value = def.substr(equal_sign + 1);
			shaderc_compile_options_add_macro_definition(options, key.c_str(), key.size(), value.c_str(), value.size());
		} else {
			shaderc_compile_options_add_macro_definition(options, def.c_str(), def.size(), nullptr, 0);
		}
	}
	shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
#ifndef NDEBUG
	shaderc_compile_options_set_generate_debug_info(options);

#endif
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, source.c_str(), source.size(), VulkanUnitConverter::ShaderStageToShadercShaderKind(type), "shader", "main", options);

	if (shaderc_result_get_num_errors(result) > 0) {
		auto message = shaderc_result_get_error_message(result);
		throw std::runtime_error(message);
	}

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pCode = (uint32_t*)shaderc_result_get_bytes(result);
	info.codeSize = shaderc_result_get_length(result);
	
	vkCreateShaderModule(context->GetVkDevice(), &info, NULL, &module);

	shaderc_result_release(result);
	shaderc_compiler_release(compiler);
	shaderc_compile_options_release(options);

	return module;
}

VulkanParsedShader VulkanShaderManager::LinkShader(VulkanParsedShader shader, const std::vector<std::string>& compiler_definitions)
{

	for (auto& shader_stage : shader) {
		VkShaderModule shader_stage_compiled = CompileShaderStage(shader_stage.type, shader_stage.source, compiler_definitions);
		shader_stage.module = shader_stage_compiled;
	}

	return std::move(shader);
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

Shader* VulkanShaderManager::CreateShaderFromString_impl(const std::string& source, const std::vector<std::string>& compiler_definitions)
{
	VulkanShader* new_shader = new VulkanShader;
	VulkanParsedShader shader_stages = LinkShader(ParseShader(source), compiler_definitions);
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
