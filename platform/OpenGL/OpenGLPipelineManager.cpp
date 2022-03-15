#include "OpenGLPipelineManager.h"
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <GL/glew.h>
#include <Profiler.h>
#include <platform/OpenGL/OpenGLShaderManager.h>
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <platform/OpenGL/OpenGLRenderDescriptorHeapBlock.h>
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
	
	OpenGLRootSignature* sig = static_cast<OpenGLRootSignature*>(signature);
	int index = sig->GetUniformBlockBindingId(semantic_name);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, static_cast<OpenGLRenderBufferResource*>(buffer.get())->GetRenderId());
}

void OpenGLPipeline::SetTexture2D(RootBinding binding_id, std::shared_ptr<RenderTexture2DResource> texture)
{
	glActiveTexture(GL_TEXTURE0 + binding_id);
	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(texture.get())->GetRenderId());
}

void OpenGLPipeline::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture)
{
	OpenGLRootSignature* sig = static_cast<OpenGLRootSignature*>(signature);
	int index = sig->GetTextureSlot(semantic_name);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(texture.get())->GetRenderId());
}

//TODO: FIX THIS, IT MIGHT WORK, BUT I DONT LIKE IT !!!!!!!!!!!!!!!!!!!!!!
void OpenGLPipeline::SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table)
{
	OpenGLRootSignature* sig = static_cast<OpenGLRootSignature*>(signature);
	int current = 0;
	auto table_desc = sig->GetTableBinding(semantic_name);
	int buf_start = table_desc.starting_binding_id;
	int tex_start = table_desc.starting_binding_id;

	for (auto entry : table_desc.table) {
		switch (entry.type) {
		case RootDescriptorType::CONSTANT_BUFFER:
			for (int i = 0; i < entry.size; i++) {
				if (static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].type == RootParameterType::CONSTANT_BUFFER) {
					SetConstantBuffer(buf_start,
						std::static_pointer_cast<RenderBufferResource>(static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].m_resource));
					i++;
					buf_start++;
				}
				else {
					throw std::runtime_error("Descriptor is not of Constant Buffer type.");
				}
			}
			break;
		case RootDescriptorType::TEXTURE_2D:
			for (int i = 0; i < entry.size; i++) {
				if (static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].type == RootParameterType::TEXTURE_2D) {
					SetTexture2D(tex_start,
						std::static_pointer_cast<RenderTexture2DResource>(static_cast<OpenGLRenderDescriptorAllocation*>(table.get())->descriptor_pointer[current].m_resource));
					i++;
					tex_start++;
				}
				else {
					throw std::runtime_error("Descriptor is not of Texture2D type.");
				}
			}
			break;
		}
		current++;
	}
}


OpenGLPipeline::OpenGLPipeline(const PipelineDescriptor& desc) : Pipeline(desc)
{

}

OpenGLPipeline::OpenGLPipeline(PipelineDescriptor&& desc) : Pipeline(std::move(desc))
{

}
