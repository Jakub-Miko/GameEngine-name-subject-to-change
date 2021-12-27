#pragma once
#include <Renderer/ShaderManager.h>
#include <unordered_map>
#include <mutex>
#include <vector>

struct ShaderSource {
	int type;
	unsigned int id = 0;
	std::string source;
};

using ParsedShader = std::vector<ShaderSource>;

class OpenGLShader : public Shader {
public:
	OpenGLShader() {}
	OpenGLShader(unsigned int program, const std::string& ref) : program_id(program), id(ref) {}

	void SetShaderProgram(unsigned int program) {
		program_id = program;
	}

	unsigned GetShaderProgram() const {
		return program_id;
	}

	std::string GetId() const {
		return id;
	}

	void SetId(const std::string& str) {
		id = str;
	}

private:
	unsigned int program_id = 0;
	std::string id = "";
};

class OpenGLShaderManager : public ShaderManager {
public:
	friend ShaderManager;

	virtual Shader* GetShader(const std::string& name) override;

private:
	OpenGLShaderManager();
	virtual ~OpenGLShaderManager() override;

	static unsigned int CompileShader(const std::string& name);
	static ParsedShader ParseShader(const std::string& source_code);
	static unsigned int CompileShaderStage(int type, const std::string& source);
	static unsigned int LinkShader(ParsedShader shader);

private:
	std::unordered_map<std::string,OpenGLShader> m_Shaders;
	std::mutex sync_mutex;

};