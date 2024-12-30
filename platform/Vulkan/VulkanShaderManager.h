#pragma once
#include <Renderer/ShaderManager.h>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>


enum class VulkanShaderStages {
	VERTEX = 0, FRAGMENT, GEOMETRY, NUM_OF_STAGES //NUM_OF_STAGES needs to be last
};

struct VulkanShaderSource {
	VulkanShaderStages type = VulkanShaderStages::NUM_OF_STAGES;
	VkShaderModule module = VkShaderModule();
	std::string source = "";
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
	virtual Shader* CreateShaderFromString_impl(const std::string& source) override;

	VulkanShaderManager();
	virtual ~VulkanShaderManager() override;

	static VulkanParsedShader CompileShader(const std::string& name);
	static VulkanParsedShader ParseShader(const std::string& source_code);
	static VkShaderModule CompileShaderStage(VulkanShaderStages type, const std::string& source);
	static VulkanParsedShader LinkShader(VulkanParsedShader shader);

private:
	std::unordered_map<std::string,VulkanShader*> m_Shaders;
	std::mutex sync_mutex;

};