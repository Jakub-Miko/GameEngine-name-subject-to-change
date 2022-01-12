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

	virtual Shader* GetShader(const std::string& name) = 0;

	virtual ~ShaderManager() {}

private:
	static ShaderManager* instance;

};