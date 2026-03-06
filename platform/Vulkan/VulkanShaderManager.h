#pragma once
#include <Renderer/ShaderManager.h>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

enum class VulkanShaderStages {
	VERTEX = 0, FRAGMENT, GEOMETRY, COMPUTE, NUM_OF_STAGES //NUM_OF_STAGES needs to be last
};

struct VulkanShaderSource {
	VulkanShaderStages type = VulkanShaderStages::NUM_OF_STAGES;
	VkShaderModule module = VkShaderModule();
	std::string source = "";
	std::string name = "";
};

using VulkanParsedShader = std::vector<VulkanShaderSource>;

class VulkanShader : public Shader {
public:
	VulkanShader() : shader_modules() {}

	std::string GetId() const {
		return id;
	}

	void ResetStage(VulkanShaderStages stage_type);
	void SetStage(VkShaderModule stage, VulkanShaderStages stage_type);
	VkShaderModule* GetStage(VulkanShaderStages stage_type);

	void SetId(const std::string& str) {
		id = str;
	}

	virtual ~VulkanShader();

	struct VulkanShaderStage {
		VkShaderModule stage;
		bool defined = false;
	};

	const std::array<VulkanShaderStage, (int)VulkanShaderStages::NUM_OF_STAGES>& GetStages() {
		return shader_modules;
	}


private:
	std::array<VulkanShaderStage, (int)VulkanShaderStages::NUM_OF_STAGES> shader_modules;
	std::string id = "";
};

class VulkanShaderManager : public ShaderManager {
public:
	friend ShaderManager;
private:

	virtual Shader* GetShader_impl(const std::string& name) override;
	virtual Shader* CreateShader_impl(const std::string& path) override;
	virtual Shader* CreateShaderFromString_impl(const std::string& source, const std::vector<std::string>& compiler_definitions = std::vector<std::string>(), const std::string& file_name = "") override;

	VulkanShaderManager();
	virtual ~VulkanShaderManager() override;

	static VulkanParsedShader CompileShader(const std::string& name);
	static VulkanParsedShader ParseShader(const std::string& source_code, const std::string& file_name);
	static VkShaderModule CompileShaderStage(VulkanShaderStages type, const std::string& source, const std::string& file_name, const std::vector<std::string>& compiler_definitions = std::vector<std::string>());
	static VulkanParsedShader LinkShader(VulkanParsedShader shader, const std::vector<std::string>& compiler_definitions = std::vector<std::string>());

private:
	std::unordered_map<std::string,VulkanShader*> m_Shaders;
	std::mutex sync_mutex;
};

class VulkanShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
public:

	struct ResultUserData {
		std::string source_name;
		std::string content;
	};

	VulkanShaderIncluder() = default;
	shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type,
		const char* requesting_source, size_t include_depth) override;
	void ReleaseInclude(shaderc_include_result* data) override;
	~VulkanShaderIncluder() override = default;
};