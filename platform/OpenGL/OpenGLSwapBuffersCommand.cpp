#include "OpenGLSwapBuffersCommand.h"
#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>

OpenGLSwapBuffersCommand::OpenGLSwapBuffersCommand()
{

}

void OpenGLSwapBuffersCommand::Execute()
{
	glfwSwapBuffers(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle());
	glClear(GL_COLOR_BUFFER_BIT);
}
