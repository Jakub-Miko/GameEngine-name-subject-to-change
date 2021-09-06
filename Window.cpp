#include "Window.h"
#include <platform/Windows/WindowsWindow.h>
#include <platform/GLFW/GlfwWindow.h>


#ifdef DirectX12
#include <platform/Windows/WindowsWindow.h>
#elif defined OpenGL
#include <platform/GLFW/GlfwWindow.h>
#endif



Window::Window(const Properties& props)
    :m_Properties(props)
{

}

Window* Window::CreateWindow(const Properties& props) {
    #ifdef DirectX12
    return new WindowsWindow(props);
    #elif defined OpenGL
    return new GlfwWindow(props);
    #else
    static_assert(false, "Wrong Window Type");
    #endif
}

