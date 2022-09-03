#include "OpenGLPipelineManager.h"
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <GL/glew.h>
#include <Profiler.h>
#include <platform/OpenGL/OpenGLShaderManager.h>
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <platform/OpenGL/OpenGLRenderDescriptorHeapBlock.h>
#include <platform/OpenGL/OpenGLUnitConverter.h>
#include <memory>
#include <Renderer/Renderer.h>

std::shared_ptr<Pipeline> OpenGLPipelineManager::CreatePipeline(const PipelineDescriptor& desc)
{
	Pipeline* pipeline = new OpenGLPipeline(desc);
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([pipeline]() {
		//Pipeline Create
		PROFILE("Pipeline Create");
		}));
	return std::shared_ptr<Pipeline>(pipeline);
}

OpenGLPipelineManager::OpenGLPipelineManager()
{

}

RootBinding OpenGLPipeline::GetBindingId(const std::string& name)
{
	return RootBinding();
}

OpenGLPipeline::~OpenGLPipeline()
{

}

void OpenGLPipeline::SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer)
{
	if (buffer->GetBufferDescriptor().usage != RenderBufferUsage::CONSTANT_BUFFER) {
		throw std::runtime_error("This buffer isn't a constant buffer");
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, binding_id, static_cast<OpenGLRenderBufferResource*>(buffer.get())->GetRenderId());
}

void OpenGLPipeline::SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer)
{
	if (buffer->GetBufferDescriptor().usage != RenderBufferUsage::CONSTANT_BUFFER) {
		throw std::runtime_error("This buffer isn't a constant buffer");
	}
	
	const OpenGLRootSignature* sig = static_cast<const OpenGLRootSignature*>(&GetSignature());
	int index = sig->GetUniformBlockBindingId(semantic_name);
	auto program = static_cast<OpenGLShader*>(GetShader().get())->GetShaderProgram();
	int location = glGetUniformBlockIndex(program, semantic_name.c_str());
	glUniformBlockBinding(program, location, index);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, static_cast<OpenGLRenderBufferResource*>(buffer.get())->GetRenderId());
}

void OpenGLPipeline::SetTexture2D(RootBinding binding_id, std::shared_ptr<RenderTexture2DResource> texture)
{
	glActiveTexture(GL_TEXTURE0 + binding_id);
	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(texture.get())->GetRenderId());
}

void OpenGLPipeline::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture)
{
	const OpenGLRootSignature* sig = static_cast<const OpenGLRootSignature*>(&GetSignature());
	int index = sig->GetTextureSlot(semantic_name);
	auto program = static_cast<OpenGLShader*>(GetShader().get())->GetShaderProgram();
	glUniform1i(glGetUniformLocation(program, semantic_name.c_str()), index);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(texture.get())->GetRenderId());
}

void OpenGLPipeline::SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture)
{
	const OpenGLRootSignature* sig = static_cast<const OpenGLRootSignature*>(&GetSignature());
	int index = sig->GetTextureSlot(semantic_name);
	auto program = static_cast<OpenGLShader*>(GetShader().get())->GetShaderProgram();
	glUniform1i(glGetUniformLocation(program, semantic_name.c_str()), index);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<OpenGLRenderTexture2DArrayResource*>(texture.get())->GetRenderId());
}

void OpenGLPipeline::SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture)
{
	const OpenGLRootSignature* sig = static_cast<const OpenGLRootSignature*>(&GetSignature());
	int index = sig->GetTextureSlot(semantic_name);
	auto program = static_cast<OpenGLShader*>(GetShader().get())->GetShaderProgram();
	glUniform1i(glGetUniformLocation(program, semantic_name.c_str()), index);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_CUBE_MAP, static_cast<OpenGLRenderTexture2DCubemapResource*>(texture.get())->GetRenderId());
}

//NOTE: Internal and for advanced use only
//Should Be used every time a new vertex buffer is bound in a different context then it was last time.
void OpenGLPipeline::BeginVertexContext(std::shared_ptr<RenderBufferResource> vertex_buffer)
{
	if (!(bool)(flags & PipelineFlags::IS_MULTI_WINDOW)) {
		throw std::runtime_error("BeginVertexContext should only be used with IS_MULTI_WINDOW pipeline flags and EndVertexContext should always be called in the same context");
	}
	if (extra_id != 0) {
		return;
	}
	auto gl_buffer = static_cast<OpenGLRenderBufferResource*>(vertex_buffer.get());
	int stride = GetLayout().stride;
	int offset = 0;
	int count = 0;
	unsigned int vao = 0;
	glGenVertexArrays(1, &vao);
	auto error = glGetError();
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer->GetRenderId());
	for (auto attrib : GetLayout().layout) {
		glEnableVertexAttribArray(count);
		glVertexAttribPointer(count, attrib.size, OpenGLUnitConverter::PrimitiveToGL(attrib.type), attrib.normalized, stride, (void*)offset);
		offset += attrib.size * OpenGLUnitConverter::PrimitiveSize(attrib.type);
		count++;
	}
	extra_id = vao;

}

//NOTE: Internal and for advanced use only
//Should always be called after BeginVertexContext when the vertex buffer is no longer going to be used, but should be called before a context switch. 
void OpenGLPipeline::EndVertexContext()
{
	if (!(bool)(flags & PipelineFlags::IS_MULTI_WINDOW)) {
		throw std::runtime_error("EndVertexContext should always be called after BeginVertexContext when the vertex buffer is no longer going to be used, but should be called before a context switch");
	}
	if (extra_id == 0) {
		return;
	}
	glDeleteVertexArrays(1, &extra_id);
	extra_id = 0;

}

//TODO: FIX THIS, IT MIGHT WORK, BUT I DONT LIKE IT !!!!!!!!!!!!!!!!!!!!!!
void OpenGLPipeline::SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table)
{
	const OpenGLRootSignature* sig = static_cast<const OpenGLRootSignature*>(&GetSignature());
	int current = 0;
	auto table_desc = sig->GetTableBinding(semantic_name);
	int buf_start = table_desc.starting_binding_id;
	int tex_start = table_desc.starting_texture_id;

	for (auto entry : table_desc.table) {
		if (entry.individual_names.empty()) throw std::runtime_error("Descriptor tables must have assigned individual_names when using OpenGL");
		switch (entry.type) {
		case RootDescriptorType::CONSTANT_BUFFER:
			for (uint32_t i = 0; i < entry.size; i++) {
				if (static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].type == RootParameterType::CONSTANT_BUFFER) {
					SetConstantBuffer(entry.individual_names[i],
						std::static_pointer_cast<RenderBufferResource>(static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].m_resource));
					//i++; // I dont think this should be here !!!!!!!!!!!!!!!
					buf_start++;
				}
				else {
					throw std::runtime_error("Descriptor is not of Constant Buffer type.");
				}
				current++;
			}
			break;
		case RootDescriptorType::TEXTURE_2D:
			for (uint32_t i = 0; i < entry.size; i++) {
				if (static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].type == RootParameterType::TEXTURE_2D) {
					SetTexture2D(entry.individual_names[i],
						std::static_pointer_cast<RenderTexture2DResource>(static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].m_resource));
					//i++; // I dont think this should be here !!!!!!!!!!!!!!!
					tex_start++;
				}
				else {
					throw std::runtime_error("Descriptor is not of Texture2D type.");
				}
				current++;
			}
			break;
		case RootDescriptorType::TEXTURE_2D_ARRAY:
			for (uint32_t i = 0; i < entry.size; i++) {
				if (static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].type == RootParameterType::TEXTURE_2D_ARRAY) {
					SetTexture2DArray(entry.individual_names[i],
						std::static_pointer_cast<RenderTexture2DArrayResource>(static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].m_resource));
					//i++; // I dont think this should be here !!!!!!!!!!!!!!!
					tex_start++;
				}
				else {
					throw std::runtime_error("Descriptor is not of Texture2DArray type.");
				}
				current++;
			}
			break;
		case RootDescriptorType::TEXTURE_2D_CUBEMAP:
			for (uint32_t i = 0; i < entry.size; i++) {
				if (static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].type == RootParameterType::TEXTURE_2D_CUBEMAP) {
					SetTexture2DCubemap(entry.individual_names[i],
						std::static_pointer_cast<RenderTexture2DCubemapResource>(static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].m_resource));
					//i++; // I dont think this should be here !!!!!!!!!!!!!!!
					tex_start++;
				}
				else {
					throw std::runtime_error("Descriptor is not of Texture2DCubemap type.");
				}
				current++;
			}
			break;
		}
	}
}


OpenGLPipeline::OpenGLPipeline(const PipelineDescriptor& desc) : Pipeline(desc)
{

}

OpenGLPipeline::OpenGLPipeline(PipelineDescriptor&& desc) : Pipeline(std::move(desc))
{

}
