#include "OpenGLShaderManager.h"
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <fstream>
#include <FileManager.h>
#include <stdexcept>
#include <GL/glew.h>
#include <sstream>

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

		return parsed;

	}
	else {
		throw std::runtime_error("Shader format unsupported");
	}
}

unsigned int OpenGLShaderManager::CompileShaderStage(int type, const std::string& source)
{
	const char* shader_src_buf = source.c_str();
	int length = source.length();
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

}

Shader* OpenGLShaderManager::GetShader(const std::string& name) {
	std::unique_lock<std::mutex> lock(sync_mutex);
	auto fnd = m_Shaders.find(name);
	if (fnd != m_Shaders.end()) {
		return static_cast<Shader*>(&fnd->second);
	}
	else {
		OpenGLShader* new_shader = &(m_Shaders.insert(std::make_pair(name, OpenGLShader())).first->second);
		lock.unlock();
		OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
		queue->ExecuteCustomCommand(new ExecutableCommandAdapter([new_shader, name]() {
			if (new_shader->GetShaderProgram() == 0) {
				new_shader->SetShaderProgram(CompileShader(name));
				new_shader->SetId(name);
			}
			}));

		return static_cast<Shader*>(new_shader);
	}
}

