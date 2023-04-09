#include "GlfwWindow.h"
#include <GL/glew.h>
#include <Renderer/Renderer.h>
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#include <GLFW/glfw3.h>
#include <ConfigManager.h>
#include <vector>
#include <Application.h>

GlfwWindow::GlfwWindow(const WindowProperties& props)
    : Window(props)
{
    if (props.resolution_x == -1 || props.resolution_y == -1) {
        m_Properties.resolution_x = ConfigManager::Get()->GetInt("resolution_X");
        m_Properties.resolution_y = ConfigManager::Get()->GetInt("resolution_Y");
        if (ConfigManager::Get()->Exists("fullscreen")) {
            m_Properties.fullscreen = (bool)ConfigManager::Get()->GetInt("fullscreen");
        }
    }
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
    
    //this causes issues on linux since it will automatically maximize all undocked windows.
    //glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

#ifdef EDITOR
    int x, y;
    const GLFWvidmode* monitor = glfwGetVideoMode(glfwGetPrimaryMonitor());

    x = monitor->width;
    y = monitor->height;

    m_Window = glfwCreateWindow(x, y, m_Properties.name.c_str(), NULL, NULL);
    glfwMaximizeWindow(m_Window);
#else 
    m_Window = glfwCreateWindow(m_Properties.resolution_x, m_Properties.resolution_y, m_Properties.name.c_str(), m_Properties.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
    
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

void GlfwWindow::DisableCursor()
{
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void GlfwWindow::EnableCursor()
{
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

#ifdef EDITOR

void GlfwWindow::AdjustWidowToDisabledEditor()
{
    const GLFWvidmode* monitor = glfwGetVideoMode(glfwGetPrimaryMonitor());
    auto& props = Application::Get()->GetWindow()->GetProperties();
    //Maximize is needed on linux to glfwRestoreWindow after fullscreen; without it the window spans the whole screen and
    //is covered by desktop panels. 
    // TODO: implement custom resolution storing and restoration
    glfwMaximizeWindow(m_Window);
    glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_FALSE);
    glfwSetWindowPos(m_Window, (monitor->width / 2) - (props.resolution_x / 2), (monitor->height / 2) - (props.resolution_y / 2));
    glfwSetWindowSize(m_Window, props.resolution_x, props.resolution_y);
    if (props.fullscreen) {
        glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, props.resolution_x, props.resolution_y, monitor->refreshRate);
    }
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void GlfwWindow::AdjustWidowToEnabledEditor()
{
    const GLFWvidmode* monitor = glfwGetVideoMode(glfwGetPrimaryMonitor());
    auto& props = Application::Get()->GetWindow()->GetProperties();
    
    glfwRestoreWindow(m_Window);
    if (props.fullscreen) {
        int width, height;
        glfwGetWindowSize(m_Window,&width, &height);
        glfwSetWindowMonitor(m_Window, NULL, (width / 2) - (props.resolution_x / 2), (height / 2) - (props.resolution_y / 2), width, height, monitor->refreshRate);
    }
    glfwMaximizeWindow(m_Window);
    glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_TRUE);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

#endif

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

