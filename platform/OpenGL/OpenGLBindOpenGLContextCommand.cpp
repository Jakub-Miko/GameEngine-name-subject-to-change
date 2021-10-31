#include "OpenGLBindOpenGLContextCommand.h"
#include <GL/glew.h>
#include <ConfigManager.h>
#include <GLFW/glfw3.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>
void OpenGLBindOpenGLContextCommand::Execute()
{
    /* Make the window's context current */
    glfwMakeContextCurrent(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle());
    glfwSwapInterval(ConfigManager::Get()->GetInt("Vsync"));

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        Application::Get()->Exit();

    }

}
