#pragma once 
#include <string>
#include <mutex>
#include <unordered_map>

class RootSignature;

class Shader {
public:
	virtual ~Shader() {}
	const RootSignature& GetRootSignature() const {
		return *signature;
	}
private:
	friend class ShaderManager;
	std::unique_ptr<RootSignature> signature;
};

class ShaderManager {
public:

	static void Initialize();
	static ShaderManager* Get();
	static void Shutdown();

	virtual ~ShaderManager() {}

	std::shared_ptr<Shader> GetShader(const std::string& path);

	std::shared_ptr<Shader> CreateShaderFromString(const std::string& shader);

private:

	RootSignature* ParseRootSignature(const std::string& signature_string);

	virtual Shader* CreateShaderFromString_impl(const std::string& source) = 0;
	virtual Shader* CreateShader_impl(const std::string& path) = 0;
	virtual Shader* GetShader_impl(const std::string& name) = 0;

	std::mutex shader_map_mutex;
	std::unordered_map<std::string, std::shared_ptr<Shader>> shader_map;

	static ShaderManager* instance;

};