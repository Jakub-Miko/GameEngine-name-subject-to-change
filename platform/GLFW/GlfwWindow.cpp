#ifdef OpenGL
#include "GlfwWindow.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Application.h>

GlfwWindow::GlfwWindow(const WindowProperties& props)
    : Window(props)
{

}

void GlfwWindow::Init()
{
    /* Make the window's context current */
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(0);
}

void GlfwWindow::PreInit()
{
    /* Initialize the library */
    if (!glfwInit())
        Application::Get()->Exit();
    
    
    
    /* Create a windowed mode window and its OpenGL context */
    m_Window = glfwCreateWindow(m_Properties.resolution_x, m_Properties.resolution_y, m_Properties.name.c_str(), NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        Application::Get()->Exit();
    }
}

void GlfwWindow::PollEvents()
{
    PROFILE("Poll Events");
    if (!glfwWindowShouldClose(m_Window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        

        /* Poll for and process events */
        glfwPollEvents();
    }
    else {
        Application::Get()->Exit();
    }
}

void GlfwWindow::SwapBuffers()
{
    glfwSwapBuffers(m_Window);
}

GlfwWindow::~GlfwWindow()
{
    glfwTerminate();
}
#endif