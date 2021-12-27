#pragma once
#include <string>

class OpenGLShaderTest {
public:
	OpenGLShaderTest();
	OpenGLShaderTest(const std::string& vertex_shader, const std::string& fragment_shader);

	OpenGLShaderTest(const OpenGLShaderTest& ref) = delete;
	OpenGLShaderTest(OpenGLShaderTest&& ref) noexcept;

	OpenGLShaderTest& operator=(const OpenGLShaderTest& ref) = delete;
	OpenGLShaderTest& operator=(OpenGLShaderTest&& ref) noexcept;

	bool CreateShader(const std::string& vertex_shader, const std::string& fragment_shader);

	unsigned int GetHandle() const { return m_ProgramHandle; };

	void Bind();
	void Unbind();

	~OpenGLShaderTest();

public:

	static OpenGLShaderTest LoadFromFile(const std::string& vertex_path, const std::string& fragment_path);

private:

	void DeleteShader();

	unsigned int CompileShader(const std::string& shader, unsigned int type);

	unsigned int LinkShader(unsigned int vertex_handle, unsigned int fragment_handle);

private:
	unsigned int m_ProgramHandle;

};