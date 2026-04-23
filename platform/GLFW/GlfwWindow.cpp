#include "GlfwWindow.h"
#include <Renderer/Renderer.h>
#ifdef OpenGL_API
#include <GL/glew.h>
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#elif defined(Vulkan_API)
#include <platform/Vulkan/VulkanRenderContext.h>
#include <platform/Vulkan/VulkanRenderSurface.h>
#include <vulkan/vulkan.h>
#endif


#include <GLFW/glfw3.h>
#include <ConfigManager.h>
#include <vector>
#include <Application.h>

GlfwWindow::GlfwWindow(const WindowProperties& props)
    : Window(props)
{
#ifdef EDITOR
    int x, y;
    const GLFWvidmode* monitor = glfwGetVideoMode(glfwGetPrimaryMonitor());

    x = monitor->width;
    y = monitor->height;

    if(m_Properties.main_window) {
        m_Window = glfwCreateWindow(x, y, m_Properties.name.c_str(), NULL, NULL);
        glfwMaximizeWindow(m_Window);
    }
    else {
        m_Window = glfwCreateWindow(m_Properties.resolution.x, m_Properties.resolution.y, m_Properties.name.c_str(), m_Properties.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
#else 
    m_Window = glfwCreateWindow(m_Properties.resolution_x, m_Properties.resolution_y, m_Properties.name.c_str(), m_Properties.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif

    glfwSetWindowUserPointer(m_Window, this);

    if (!m_Window)
    {
        throw std::runtime_error("Window could not be created.");
    }
}

void GlfwWindow::InitSurface() {
    VkSurfaceKHR vk_surface;
    DEFINE_VK_INSTANCE(context);
    if (glfwCreateWindowSurface(context->GetVkInstance(), m_Window, NULL, &vk_surface)) {
        throw std::runtime_error("Surface could not be created from glfwWindow");
    }
    window_render_surface = std::make_shared<VulkanRenderSurface>(weak_from_this(), vk_surface, true);
}

void GlfwWindow::Init()
{
    if (!glfwInit())
        Application::Get()->Exit();
#ifdef Vulkan_API
    VulkanRenderContext* context = static_cast<VulkanRenderContext*>(RenderContext::Get());

    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    context->RequestExtensions(extensions, count);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#endif
}

void GlfwWindow::Shutdown()
{
    glfwTerminate();
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
    //glfwSwapBuffers(m_Window);
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

bool GlfwWindow::IsMinimized() {
    bool iconified = glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
    auto res = GetFramebufferResolution();
    return iconified || res.x == 0 || res.y == 0;
}

glm::ivec2 GlfwWindow::GetFramebufferResolution() {
    int x = 0, y = 0;
    glfwGetFramebufferSize(m_Window, &x, &y);
    return { x, y };
}

#ifdef EDITOR

void GlfwWindow::AdjustWidowToDisabledEditor()
{
    const GLFWvidmode* monitor = glfwGetVideoMode(glfwGetPrimaryMonitor());

    //Maximize is needed on linux to glfwRestoreWindow after fullscreen; without it the window spans the whole screen and
    //is covered by desktop panels. 
    // TODO: implement custom resolution storing and restoration

    glm::vec2 render_res = Renderer3D::Get()->GetRenderResolution();
    glfwMaximizeWindow(m_Window);
    glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_FALSE);
    glfwSetWindowPos(m_Window, (monitor->width / 2) - (render_res.x / 2), (monitor->height / 2) - (render_res.y / 2));
    glfwSetWindowSize(m_Window, render_res.x, render_res.y);
    if (m_Properties.fullscreen) {
        glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, render_res.x, render_res.y, monitor->refreshRate);
    }
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void GlfwWindow::AdjustWidowToEnabledEditor()
{
    const GLFWvidmode* monitor = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glm::vec2 render_res = Renderer3D::Get()->GetRenderResolution();

    glfwRestoreWindow(m_Window);
    if (m_Properties.fullscreen) {
        int width, height;
        glfwGetWindowSize(m_Window,&width, &height);
        glfwSetWindowMonitor(m_Window, NULL, (width / 2) - (render_res.x / 2), (height / 2) - (render_res.y / 2), width, height, monitor->refreshRate);
    }
    glfwMaximizeWindow(m_Window);
    glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_TRUE);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}


#endif

GlfwWindow::~GlfwWindow()
{
    glfwDestroyWindow(m_Window);
}

void GlfwWindow::DropCallback(GLFWwindow* window, int count, const char** paths)
{
    std::vector<std::string> paths_vec;
    for (int i = 0; i < count; i++) {
        paths_vec.push_back(paths[i]);
    }

    std::static_pointer_cast<GlfwWindow>(Application::Get()->GetWindow())->drop_callback(count, paths_vec);
}

void GlfwWindow::DefaultDropCallback(int count, std::vector<std::string> paths)
{
}

