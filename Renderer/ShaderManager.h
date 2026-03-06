#pragma once 
#include <string>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <vector>

class RootSignature;
class Material;

class Shader {
public:
	virtual ~Shader();
	const RootSignature& GetRootSignature() const {
		return *signature;
	}

	const std::string& GetPath() const {
		return path;
	}

	const std::vector<std::string>& GetCompilerDefinitions() const {
		return compiler_definitions;
	}

private:
	friend class ShaderManager;
	std::string path = "";
	std::vector<std::string> compiler_definitions;
	std::unique_ptr<RootSignature> signature;
};

class ShaderManager {
public:

	static void Initialize();
	static ShaderManager* Get();
	static void Shutdown();

	virtual ~ShaderManager();

	std::shared_ptr<Shader> GetShader(const std::string& path, const std::vector<std::string>& compiler_definitions = std::vector<std::string>());

	std::shared_ptr<Shader> CreateShaderFromString(const std::string& shader, const std::vector<std::string>& compiler_definitions = std::vector<std::string>());

private:

	RootSignature* ParseRootSignature(const std::string& signature_string);

	virtual Shader* CreateShaderFromString_impl(const std::string& source, const std::vector<std::string>& compiler_definitions, const std::string& file_name = "") = 0;
	virtual Shader* CreateShader_impl(const std::string& path) = 0;
	virtual Shader* GetShader_impl(const std::string& name) = 0;

	std::mutex shader_map_mutex;
	std::unordered_map<std::string, std::shared_ptr<Shader>> shader_map;

	static ShaderManager* instance;

};