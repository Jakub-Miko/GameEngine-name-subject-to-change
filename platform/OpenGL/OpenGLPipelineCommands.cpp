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
	
	RenderViewport viewport = pipeline->GetViewport();
	RenderScissorRect scissor = pipeline->GetScissorRect();
	const WindowProperties& props = Application::Get()->GetWindow()->GetProperties();
	const PipelineFlags& flags = pipeline->GetPipelineFlags();
	unsigned int shader = static_cast<const OpenGLShader*>(pipeline->GetShader())->GetShaderProgram();
	
	if (!current_shader || current_shader->GetShaderProgram() != shader) {
		glUseProgram(shader);
		queue->SetShader(pipeline->GetShader());
	}
	
	if (viewport.size == glm::vec2(-1, -1)) {
		viewport.size = { props.resolution_x,props.resolution_y };
	};
	if (scissor.size == glm::vec2(-1, -1)) {
		scissor.size = { props.resolution_x,props.resolution_y };
	}
	
	if (current_state.GetViewport() != viewport) {
		glViewport(viewport.offset.x, viewport.offset.y, viewport.size.x, viewport.size.y);
		glDepthRange(viewport.min_depth, viewport.max_depth);
		queue->SetViewport(viewport);
	}
	
	if (current_state.scissor_rect != scissor) {
		glScissor(scissor.offset.x, scissor.offset.y, scissor.size.x, scissor.size.y);
		queue->SetScissorRect(scissor);
	}

	if (current_state.flags != flags) {
		if (uint32_t(flags & PipelineFlags::ENABLE_DEPTH_TEST)) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);

		if (uint32_t(flags & PipelineFlags::ENABLE_SCISSOR_TEST)) glEnable(GL_SCISSOR_TEST);
		else glDisable(GL_SCISSOR_TEST);

		if (uint32_t(flags & PipelineFlags::ENABLE_STENCIL_TEST)) glEnable(GL_STENCIL_TEST);
		else glDisable(GL_STENCIL_TEST);
		queue->SetFlags(flags);
	}

}
