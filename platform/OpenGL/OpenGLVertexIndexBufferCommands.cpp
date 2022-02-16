#include "OpenGLVertexIndexBufferCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/OpenGL/OpenGLPipelineManager.h>
#include <stdexcept>
#include "OpenGLUnitConverter.h"
#include <Profiler.h>

void OpenGLSetVertexBufferCommand::Execute()
{
	PROFILE("Set Vertex Buffer");
	OpenGLRenderBufferResource* gl_buffer = static_cast<OpenGLRenderBufferResource*>(buffer.get());
	if (buffer->GetBufferDescriptor().usage != RenderBufferUsage::VERTEX_BUFFER) {
		throw std::runtime_error("The Buffer must be initialized as an vertex buffer");
	}
	if (gl_buffer->GetVAOId() != 0) {
		glBindVertexArray(gl_buffer->GetVAOId());
	}
	else {
		OpenGLPipeline* gl_pipeline = static_cast<OpenGLPipeline*>(pipeline);
		int stride = gl_pipeline->GetLayout().stride;
		int offset = 0;
		int count = 0;
		unsigned int vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, gl_buffer->GetRenderId());
		for (auto attrib : gl_pipeline->GetLayout().layout) {
			glEnableVertexAttribArray(count);
			glVertexAttribPointer(count, attrib.size, OpenGLUnitConverter::PrimitiveToGL(attrib.type),  GL_FALSE, stride, (void*)offset);
			offset += attrib.size * OpenGLUnitConverter::PrimitiveSize(attrib.type);
			count++;
		}
		gl_buffer->SetVAOId(vao);
	}

}
