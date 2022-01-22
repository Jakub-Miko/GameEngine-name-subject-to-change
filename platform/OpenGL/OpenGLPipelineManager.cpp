#include "OpenGLPipelineManager.h"
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <GL/glew.h>
#include <Profiler.h>
#include <platform/OpenGL/OpenGLShaderManager.h>
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <Renderer/Renderer.h>

Pipeline* OpenGLPipelineManager::CreatePipeline(const PipelineDescriptor& desc)
{
	Pipeline* pipeline = new OpenGLPipeline(desc);
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	queue->ExecuteCustomCommand(new ExecutableCommandAdapter([pipeline]() {
		//Pipeline Create
		PROFILE("Pipeline Create");
		}));
	return pipeline;
}

OpenGLPipelineManager::OpenGLPipelineManager()
{

}

RootBinding OpenGLPipeline::GetBindingId(const std::string& name)
{
	return RootBinding();
}

void OpenGLPipeline::SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer)
{
	if (buffer->GetBufferDescriptor().usage != RenderBufferUsage::CONSTANT_BUFFER) {
		throw std::runtime_error("This buffer isn't a constant buffer");
	}
	glUniformBlockBinding(static_cast<OpenGLShader*>(shader)->GetShaderProgram(), binding_id, static_cast<OpenGLRenderBufferResource*>(buffer.get())->GetRenderId());
}

void OpenGLPipeline::SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer)
{
	unsigned int id = glGetUniformBlockIndex(static_cast<OpenGLShader*>(shader)->GetShaderProgram(), semantic_name.c_str());

	if (buffer->GetBufferDescriptor().usage != RenderBufferUsage::CONSTANT_BUFFER) {
		throw std::runtime_error("This buffer isn't a constant buffer");
	}
	
	if (id != -1) {
		OpenGLRootSignature* sig = static_cast<OpenGLRootSignature*>(signature);
		int index = sig->GetUniformBlockBindingId(semantic_name);
		glUniformBlockBinding(static_cast<OpenGLShader*>(shader)->GetShaderProgram(), id, index);
		glBindBufferBase(GL_UNIFORM_BUFFER, index, static_cast<OpenGLRenderBufferResource*>(buffer.get())->GetRenderId());
	}
	else {
		throw std::runtime_error("Constant buffer with name " + semantic_name + " doesn't exist.");
	}
}

void OpenGLPipeline::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture)
{
	OpenGLRootSignature* sig = static_cast<OpenGLRootSignature*>(signature);
	int index = sig->GetTextureSlot(semantic_name);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(texture.get())->GetRenderId());
}


OpenGLPipeline::OpenGLPipeline(const PipelineDescriptor& desc) : Pipeline(desc)
{

}

OpenGLPipeline::OpenGLPipeline(PipelineDescriptor&& desc) : Pipeline(std::move(desc))
{

}
