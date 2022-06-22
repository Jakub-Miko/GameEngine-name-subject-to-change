#include "GlfwWindow.h"
#include <GL/glew.h>
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <Application.h>

GlfwWindow::GlfwWindow(const WindowProperties& props)
    : Window(props)
{

}

void GlfwWindow::Init()
{
    auto list = reinterpret_cast<OpenGLRenderCommandList*>(Renderer::Get()->GetRenderCommandList());
    list->BindOpenGLContext();
    std::shared_ptr<RenderFence> fence = std::shared_ptr<RenderFence>( Renderer::Get()->GetFence());
    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList( reinterpret_cast<RenderCommandList*>(list) );
    Renderer::Get()->GetCommandQueue()->Signal(fence, 1);
    fence->WaitForValue(1);
    glfwSetDropCallback(m_Window, &DropCallback);
    RegistorDragAndDropCallback(&DefaultDropCallback);
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

void GlfwWindow::RegistorDragAndDropCallback(void(*callback)(int count, std::vector<std::string>paths))
{
    drop_callback = callback;
}


GlfwWindow::~GlfwWindow()
{
    glfwTerminate();
}

void GlfwWindow::DropCallback(GLFWwindow* window, int count, const char** paths)
{
    std::vector<std::string> paths_vec;
    for (int i = 0; i < count; i++) {
        paths_vec.push_back(paths[i]);
    }

    static_cast<GlfwWindow*>(Application::Get()->GetWindow())->drop_callback(count, paths_vec);
}

void GlfwWindow::DefaultDropCallback(int count, std::vector<std::string> paths)
{
}

