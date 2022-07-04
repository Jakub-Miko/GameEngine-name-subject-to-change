#include "OpenGLBindOpenGLContextCommand.h"
#include <GL/glew.h>
#include <ConfigManager.h>
#include <GLFW/glfw3.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>

static void GLAPIENTRY
    MessageCallback(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar * message,
        const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM || type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    }
}

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


    // During init, enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}
