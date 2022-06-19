#include "OpenGLPresent.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>

void OpenGLPresent::Execute()
{
	PROFILE("Present");
	glfwSwapBuffers(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}
