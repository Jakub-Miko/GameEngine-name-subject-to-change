#ifdef Glfw_Window
#include "GlfwWindow.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Application.h>

void GlfwWindow::Init()
{
    /* Make the window's context current */
    glfwMakeContextCurrent(m_Window);
}

void GlfwWindow::PreInit()
{
    /* Initialize the library */
    if (!glfwInit())
        Application::Get()->Exit();
    
    /* Create a windowed mode window and its OpenGL context */
    m_Window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        Application::Get()->Exit();
    }
}

void GlfwWindow::PollEvents()
{
    if (!glfwWindowShouldClose(m_Window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(m_Window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    else {
        Application::Get()->Exit();
    }
}

GlfwWindow::~GlfwWindow()
{
    glfwTerminate();
}
#endif