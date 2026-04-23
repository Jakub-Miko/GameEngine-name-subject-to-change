#pragma once 
#include <GLFW/glfw3.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Renderer/RenderSurface.h>
#include <memory>
class ImGuiViewport;

struct ImGui_ImplGlfw_ViewportData_internal
{
    GLFWwindow* Window;
    bool        WindowOwned;
    int         IgnoreWindowPosEventFrame;
    int         IgnoreWindowSizeEventFrame;
    
    ImGui_ImplGlfw_ViewportData_internal() { Window = NULL; WindowOwned = false; IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1; }
    ~ImGui_ImplGlfw_ViewportData_internal() { }
};

struct WindowOwnership {
    std::shared_ptr<GlfwWindow> window;
};

class impl_custom_imgui_platform {
public:

	static void init_custom_imgui_platform();
	static void shutdown_custom_imgui_platform();

	static void UpdatePlatformWindows();
private:
	static void ImGui_custom_CreateWindow(ImGuiViewport* viewport);
		
};