#include "OpenGLShaderManager.h"
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <fstream>
#include <FileManager.h>
#include <stdexcept>
#include <GL/glew.h>
#include <sstream>
#include <cstring>

OpenGLShaderManager::OpenGLShaderManager() {

}

unsigned int OpenGLShaderManager::CompileShader(const std::string& name)
{
	std::ifstream file_in(FileManager::Get()->GetRenderApiAssetFilePath("/shaders/" + name));
	std::string source;
	if (file_in.is_open()) {
		std::stringstream stream;
		stream << file_in.rdbuf();
		source = stream.str();
		return LinkShader(ParseShader(source));
	}
	else {
		throw std::runtime_error("Couldn't find the shader");
	}
}

ParsedShader OpenGLShaderManager::ParseShader(const std::string& source_code)
{
	auto fnd_vertex = source_code.find("#Vertex");
	auto fnd_fragment = source_code.find("#Fragment");
	auto fnd_geometry = source_code.find("#Geometry");
	if (fnd_vertex != source_code.npos && fnd_fragment != source_code.npos) {
		auto end_vertex = source_code.find("#end", fnd_vertex) ;
		auto end_fragment = source_code.find("#end", fnd_fragment);
		fnd_vertex += strlen("#Vertex");
		fnd_fragment += strlen("#Fragment");
		ParsedShader parsed;

		ShaderSource vertex;
		vertex.type = GL_VERTEX_SHADER;
		vertex.source = source_code.substr(fnd_vertex, end_vertex - fnd_vertex);
		parsed.push_back(vertex);

		ShaderSource fragment;
		fragment.type = GL_FRAGMENT_SHADER;
		fragment.source = source_code.substr(fnd_fragment, end_fragment - fnd_fragment);
		parsed.push_back(fragment);

		if (fnd_geometry != source_code.npos) {
			fnd_geometry += strlen("#Geometry");
			auto end_geometry = source_code.find("#end", fnd_geometry);
			ShaderSource geometry;
			geometry.type = GL_GEOMETRY_SHADER;
			geometry.source = source_code.substr(fnd_geometry, end_geometry - fnd_geometry);
			parsed.push_back(geometry);
		}

		return parsed;

	}
	else {
		throw std::runtime_error("Shader format unsupported");
	}
}

unsigned int OpenGLShaderManager::CompileShaderStage(int type, const std::string& source)
{
	const char* shader_src_buf = source.c_str();
	int length = (int)source.length();
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

unsigned int OpenGLShaderManager::LinkShader(ParsedShader shader)
{
	unsigned int program = glCreateProgram();
	for (auto& shader_stage : shader) {
		unsigned int shader_stage_compiled = CompileShaderStage(shader_stage.type, shader_stage.source);
		shader_stage.id = shader_stage_compiled;
		glAttachShader(program, shader_stage_compiled);

	}
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

	for (auto& shader_stage : shader) {
		if (shader_stage.id) {
			glDeleteShader(shader_stage.id);
		}
	}

	return program;
}

OpenGLShaderManager::~OpenGLShaderManager() {
	for (auto shader : m_Shaders) {
		delete shader.second;
	}
}

Shader* OpenGLShaderManager::GetShader_impl(const std::string& name) {
	std::unique_lock<std::mutex> lock(sync_mutex);
	auto fnd = m_Shaders.find(name);
	if (fnd != m_Shaders.end()) {
		return static_cast<Shader*>(fnd->second);
	}
	else {
		OpenGLShader* new_shader = (m_Shaders.insert(std::make_pair(name, static_cast<OpenGLShader*>(CreateShader_impl(name)))).first->second);
		lock.unlock();
		return static_cast<Shader*>(new_shader);
	}
}

Shader* OpenGLShaderManager::CreateShader_impl(const std::string& path)
{
	OpenGLShader* new_shader = new OpenGLShader;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([new_shader, path]() {
		if (new_shader->GetShaderProgram() == 0) {
			new_shader->SetShaderProgram(CompileShader(path));
			new_shader->SetId(path);
		}
		}));

	return static_cast<Shader*>(new_shader);
}

Shader* OpenGLShaderManager::CreateShaderFromString_impl(const std::string& source)
{
	OpenGLShader* new_shader = new OpenGLShader;
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([new_shader, source]() {
		if (new_shader->GetShaderProgram() == 0) {
			new_shader->SetShaderProgram(LinkShader(ParseShader(source)));
			new_shader->SetId("Unknown");
		}
		}));

	return static_cast<Shader*>(new_shader);
}

