#pragma once 
#include <string>
#include <mutex>
#include <unordered_map>
#include <memory>

class RootSignature;
class Material;

class Shader {
public:
	virtual ~Shader();
	const RootSignature& GetRootSignature() const {
		return *signature;
	}

	std::shared_ptr<Material> GetDefaultMaterial() const {
		return default_material;
	}

	const std::string& GetPath() const {
		return path;
	}

private:
	friend class ShaderManager;
	std::string path = "";
	std::unique_ptr<RootSignature> signature;
	std::shared_ptr<Material> default_material = nullptr;
};

class ShaderManager {
public:

	static void Initialize();
	static ShaderManager* Get();
	static void Shutdown();

	virtual ~ShaderManager();

	std::shared_ptr<Shader> GetShader(const std::string& path);

	std::shared_ptr<Shader> CreateShaderFromString(const std::string& shader);

private:

	RootSignature* ParseRootSignature(const std::string& signature_string, bool* has_default_material = nullptr);
	std::shared_ptr<Material> ParseDefaulMaterial(const std::string& signature_string,std::shared_ptr<Shader> shader);

	virtual Shader* CreateShaderFromString_impl(const std::string& source) = 0;
	virtual Shader* CreateShader_impl(const std::string& path) = 0;
	virtual Shader* GetShader_impl(const std::string& name) = 0;

	std::mutex shader_map_mutex;
	std::unordered_map<std::string, std::shared_ptr<Shader>> shader_map;

	static ShaderManager* instance;

};