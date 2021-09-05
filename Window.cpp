#include "Window.h"
#include <platform/Windows/WindowsWindow.h>
#include <platform/GLFW/GlfwWindow.h>


#ifdef WIN32Window
#include <platform/Windows/WindowsWindow.h>
#elif defined Glfw_Window
#include <platform/GLFW/GlfwWindow.h>
#endif



Window* Window::CreateWindow() {
    #ifdef WIN32Window
    return new WindowsWindow;
    #elif defined Glfw_Window
    return new GlfwWindow;
    #else
    static_assert(false, "Wrong Window Type");
    #endif
}

