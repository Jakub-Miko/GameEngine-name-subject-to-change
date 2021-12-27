#include "ShaderManager.h"
#include <platform/OpenGL/OpenGLShaderManager.h>

ShaderManager* ShaderManager::instance = nullptr;

void ShaderManager::Initialize()
{
	if (!instance) {
		instance = new OpenGLShaderManager();
	}
}

ShaderManager* ShaderManager::Get()
{
	return instance;
}

void ShaderManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}
