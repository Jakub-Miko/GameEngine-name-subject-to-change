#include "Window.h"
#include <platform/Windows/WindowsWindow.h>
#include <platform/GLFW/GlfwWindow.h>


Window::Window(const WindowProperties& props)
    :m_Properties(props)
{

}

void Window::Init() {

#if defined Vulkan_API
    GlfwWindow::Init();
#endif

}
 
void Window::Shutdown() {

#if defined Vulkan_API
    GlfwWindow::Shutdown();
#endif

}

std::shared_ptr<Window> Window::CreateWindow(const WindowProperties& props) {
    #ifdef DirectX12
    return new WindowsWindow(props);
    #elif defined OpenGL_API
    return std::make_shared<GlfwWindow>(props);
    #elif defined Vulkan_API
    return std::make_shared<GlfwWindow>(props);
    #else
    static_assert(false, "Wrong Window Type");
    #endif
}
