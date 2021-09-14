#include "OpenGLShader.h"
#include <GL/glew.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

OpenGLShader::OpenGLShader()
	:m_ProgramHandle(0)
{
}

OpenGLShader::OpenGLShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
	CreateShader(vertex_shader, fragment_shader);
}

OpenGLShader::OpenGLShader(OpenGLShader&& ref) noexcept
	:m_ProgramHandle(ref.m_ProgramHandle)
{
	ref.m_ProgramHandle = 0;
}

OpenGLShader& OpenGLShader::operator=(OpenGLShader&& ref) noexcept
{
	DeleteShader();
	m_ProgramHandle = ref.m_ProgramHandle;
	ref.m_ProgramHandle = 0;
	return *this;
}

bool OpenGLShader::CreateShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
	unsigned int vertex_shader_handle;
	unsigned int fragment_shader_handle;
	unsigned int program;
	try {
		vertex_shader_handle = CompileShader(vertex_shader, GL_VERTEX_SHADER);
		fragment_shader_handle = CompileShader(fragment_shader, GL_FRAGMENT_SHADER);
	}
	catch (const std::runtime_error& e) {
		std::cout << e.what() << "\n";
		return false;
	}

	try {
		program = LinkShader(vertex_shader_handle, fragment_shader_handle);
	}
	catch (const std::runtime_error& e) {
		glDeleteShader(vertex_shader_handle);
		glDeleteShader(fragment_shader_handle);
		std::cout << e.what() << "\n";
		return false;
	} 
	glDeleteShader(vertex_shader_handle);
	glDeleteShader(fragment_shader_handle);
	m_ProgramHandle = program;
	return true;
}

void OpenGLShader::Bind()
{
	if (m_ProgramHandle) {
		glUseProgram(m_ProgramHandle);
	}
}

void OpenGLShader::Unbind()
{
	glUseProgram(0);
}

OpenGLShader::~OpenGLShader()
{
	DeleteShader();
}

OpenGLShader OpenGLShader::LoadFromFile(const std::string& vertex_path, const std::string& fragment_path)
{
	std::ifstream file;
	std::string vertex_shader_src;
	std::string fragment_shader_src;
	file.open(vertex_path);
	if (file.is_open()) {
		std::stringstream stream;
		stream << file.rdbuf();
		vertex_shader_src = stream.str();
		file.close();
		file.open(fragment_path);
		if (file.is_open()) {
			std::stringstream stream;
			stream << file.rdbuf();
			fragment_shader_src = stream.str();
			
			OpenGLShader shader(vertex_shader_src, fragment_shader_src);
			file.close();
			return shader;
		}
	}
	file.close();
	throw std::runtime_error(std::string("Invalid Path: \n") + vertex_path + "\n" + fragment_path + "\n");


}

void OpenGLShader::DeleteShader() {
	if (m_ProgramHandle) {
		glDeleteProgram(m_ProgramHandle);
	}
}

unsigned int OpenGLShader::CompileShader(const std::string& shader, unsigned int type)
{
	const char* shader_src_buf = shader.c_str();
	int length = shader.length();
	unsigned int Shader = glCreateShader(type);
	glShaderSource(Shader, 1, &shader_src_buf, &length);
	glCompileShader(Shader);
	int status;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		int error_length;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &error_length);
		char* error_buf = (char*)alloca(error_length);
		glGetShaderInfoLog(Shader, error_length, nullptr, error_buf);
		glDeleteShader(Shader);
		throw std::runtime_error(std::string("Shader compilation failed.\n") + error_buf + "\n");
	}
	
	return Shader;
}

unsigned int OpenGLShader::LinkShader(unsigned int vertex_handle, unsigned int fragment_handle)
{
	unsigned int program = glCreateProgram();
	
	glAttachShader(program, vertex_handle);
	glAttachShader(program, fragment_handle);

	glLinkProgram(program);
	int status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		int error_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &error_length);
		char* error_buf = (char*)alloca(error_length);
		glGetProgramInfoLog(program, error_length, nullptr, error_buf);
		glDeleteProgram(program);
		throw std::runtime_error(std::string("Program linking failed.\n") + error_buf + "\n");
	}

	return program;
}
