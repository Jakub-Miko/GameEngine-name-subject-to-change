#include "OpenGLPipelineCommands.h"
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLShaderManager.h>
#include "OpenGLUnitConverter.h"
#include <Renderer/RendererDefines.h>
#include <Application.h>
#include <Window.h>
#include <Profiler.h>

void OpenGLSetPipelineCommand::Execute()
{
	glUseProgram(static_cast<const OpenGLShader*>(pipeline->GetShader())->GetShaderProgram());

	RenderViewport viewport = pipeline->GetViewport();
	RenderScissorRect scissor = pipeline->GetScissorRect();
	const WindowProperties& props = Application::Get()->GetWindow()->GetProperties();
	const PipelineFlags& flags = pipeline->GetPipelineFlags();
	
	if (viewport.size == glm::vec2(-1, -1)) {
		viewport.size = { props.resolution_x,props.resolution_y };
	}
	if (scissor.size == glm::vec2(-1, -1)) {
		scissor.size = { props.resolution_x,props.resolution_y };
	}
	
	glViewport(viewport.offset.x, viewport.offset.y, viewport.size.x, viewport.size.y);
	glDepthRange(viewport.min_depth, viewport.max_depth);
	glScissor(scissor.offset.x, scissor.offset.y, scissor.size.x, scissor.size.y);

	if (uint32_t(flags & PipelineFlags::ENABLE_DEPTH_TEST)) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);

	if (uint32_t(flags & PipelineFlags::ENABLE_SCISSOR_TEST)) glEnable(GL_SCISSOR_TEST);
	else glDisable(GL_SCISSOR_TEST);

	if (uint32_t(flags & PipelineFlags::ENABLE_STENCIL_TEST)) glEnable(GL_STENCIL_TEST);
	else glDisable(GL_STENCIL_TEST);
}
