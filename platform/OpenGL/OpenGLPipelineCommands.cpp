#include "OpenGLPipelineCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLShaderManager.h>
#include "OpenGLUnitConverter.h"
#include <Renderer/RendererDefines.h>
#include <Application.h>
#include <Window.h>
#include <Profiler.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>


void OpenGLSetPipelineCommand::Execute()
{
	
	PROFILE("Set Pipeline");


	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	const PipelineState& current_state = queue->GetPipelineState();
	const OpenGLShader* current_shader = static_cast<const OpenGLShader*>(current_state.GetShader());
	auto blend_func = pipeline->GetPipelineBlendFunctions();
	
	RenderViewport viewport = pipeline->GetViewport();
	RenderScissorRect scissor = pipeline->GetScissorRect();
	PrimitivePolygonRenderMode rendermode = pipeline->GetPrimitivePolygonRenderMode();
	const WindowProperties& props = Application::Get()->GetWindow()->GetProperties();
	const PipelineFlags& flags = pipeline->GetPipelineFlags();
	unsigned int shader = static_cast<const OpenGLShader*>(pipeline->GetShader())->GetShaderProgram();

	if (current_state.flags != flags || (bool)(pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
		if (uint32_t(flags & PipelineFlags::ENABLE_DEPTH_TEST)) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);

		if (uint32_t(flags & PipelineFlags::ENABLE_SCISSOR_TEST)) glEnable(GL_SCISSOR_TEST);
		else glDisable(GL_SCISSOR_TEST);

		if (uint32_t(flags & PipelineFlags::ENABLE_BLEND)) glEnable(GL_BLEND);
		else glDisable(GL_BLEND);

		if (uint32_t(flags & PipelineFlags::ENABLE_STENCIL_TEST)) glEnable(GL_STENCIL_TEST);
		else glDisable(GL_STENCIL_TEST);
		queue->SetFlags(flags);
	}
	
	if (current_state.GetPipelineBlendFunctions() != blend_func || (bool)(pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
		glBlendFuncSeparate(OpenGLUnitConverter::BlendFunctiontoGLenum(blend_func.srcRGB),
			OpenGLUnitConverter::BlendFunctiontoGLenum(blend_func.dstRGB),
			OpenGLUnitConverter::BlendFunctiontoGLenum(blend_func.srcAlpha),
			OpenGLUnitConverter::BlendFunctiontoGLenum(blend_func.dstAlpha));
		queue->SetBlendFunctions(blend_func);
	}

	if (current_state.GetPrimitivePolygonRenderMode() != rendermode || (bool)(pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
		glPolygonMode(GL_FRONT_AND_BACK, OpenGLUnitConverter::PrimitivePolygonRenderModetoGLenum(rendermode));
		queue->SetPrimitivePolygonRenderMode(rendermode);
	}

	if (!current_shader || current_shader->GetShaderProgram() != shader || (bool)(pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
		glUseProgram(shader);
		queue->SetShader(pipeline->GetShader());
	}

	if (viewport.size == glm::vec2(-1, -1)) {
		viewport.size = { props.resolution_x,props.resolution_y };
	};
	if (scissor.size == glm::vec2(-1, -1)) {
		scissor.size = { props.resolution_x,props.resolution_y };
	}
	
	if (current_state.GetViewport() != viewport || (bool)(pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
		glViewport(viewport.offset.x, viewport.offset.y, viewport.size.x, viewport.size.y);
		glDepthRange(viewport.min_depth, viewport.max_depth);
		queue->SetViewport(viewport);
	}
	
	if (current_state.scissor_rect != scissor || (bool)(pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
		glScissor(scissor.offset.x, scissor.offset.y, scissor.size.x, scissor.size.y);
		queue->SetScissorRect(scissor);
	}

}

void OpenGLSetScissorRect::Execute()
{
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	const PipelineState& current_state = queue->GetPipelineState();
	if (current_state.scissor_rect != rect) {
		glScissor(rect.offset.x, rect.offset.y, rect.size.x, rect.size.y);
		queue->SetScissorRect(rect);
	}
}

void OpenGLSetViewport::Execute()
{
	OpenGLRenderCommandQueue* queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
	const PipelineState& current_state = queue->GetPipelineState();
	if (current_state.GetViewport() != viewport) {
		glViewport(viewport.offset.x, viewport.offset.y, viewport.size.x, viewport.size.y);
		glDepthRange(viewport.min_depth, viewport.max_depth);
		queue->SetViewport(viewport);
	}
}
