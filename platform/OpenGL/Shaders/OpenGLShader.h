#pragma once
#include <string>

class OpenGLShader {
public:
	OpenGLShader();
	OpenGLShader(const std::string& vertex_shader, const std::string& fragment_shader);

	OpenGLShader(const OpenGLShader& ref) = delete;
	OpenGLShader(OpenGLShader&& ref) noexcept;

	OpenGLShader& operator=(const OpenGLShader& ref) = delete;
	OpenGLShader& operator=(OpenGLShader&& ref) noexcept;

	bool CreateShader(const std::string& vertex_shader, const std::string& fragment_shader);

	unsigned int GetHandle() const { return m_ProgramHandle; };

	void Bind();
	void Unbind();

	~OpenGLShader();

public:

	static OpenGLShader LoadFromFile(const std::string& vertex_path, const std::string& fragment_path);

private:

	void DeleteShader();

	unsigned int CompileShader(const std::string& shader, unsigned int type);

	unsigned int LinkShader(unsigned int vertex_handle, unsigned int fragment_handle);

private:
	unsigned int m_ProgramHandle;

};