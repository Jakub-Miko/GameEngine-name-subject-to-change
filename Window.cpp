#include "Window.h"
#include <platform/Windows/WindowsWindow.h>
#include <platform/GLFW/GlfwWindow.h>


Window::Window(const WindowProperties& props)
    :m_Properties(props)
{

}

Window* Window::CreateWindow(const WindowProperties& props) {
    #ifdef DirectX12
    return new WindowsWindow(props);
    #elif defined OpenGL
    return new GlfwWindow(props);
    #else
    static_assert(false, "Wrong Window Type");
    #endif
}

