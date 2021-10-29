#include "GlfwWindow.h"
#include <GL/glew.h>
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#include <GLFW/glfw3.h>
#include <Application.h>

GlfwWindow::GlfwWindow(const WindowProperties& props)
    : Window(props)
{

}

void GlfwWindow::Init()
{
    auto list = reinterpret_cast<OpenGLRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
    list->BindOpenGLContext();
    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList( reinterpret_cast<RenderCommandList*>(list) );
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

        glClear(GL_COLOR_BUFFER_BIT);

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