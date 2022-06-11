#pragma once 
#include <string>

class Shader {
public:
	virtual ~Shader() {}
};

class ShaderManager {
public:

	static void Initialize();
	static ShaderManager* Get();
	static void Shutdown();

	virtual Shader* CreateShaderFromString(const std::string& source) = 0;
	virtual Shader* CreateShader(const std::string& path) = 0;
	virtual Shader* GetShader(const std::string& name) = 0;

	virtual ~ShaderManager() {}

private:
	static ShaderManager* instance;

};