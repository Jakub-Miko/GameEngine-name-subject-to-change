#include "impl_custom_imgui_platform.h"
#include <imgui.h>
#include <Renderer/Renderer.h>

#ifdef OpenGL
#include <GLFW/glfw3.h>
#include <dependencies/imgui/backends/imgui_impl_glfw.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <platform/OpenGL/OpenGLRenderCommand.h>

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window
#endif

#define GLFW_HAS_FOCUS_ON_SHOW 1

enum GlfwClientApi
{
    GlfwClientApi_Unknown,
    GlfwClientApi_OpenGL,
    GlfwClientApi_Vulkan
};


struct ImGui_ImplGlfw_Data_internal
{
    GLFWwindow* Window;
    GlfwClientApi           ClientApi;
    double                  Time;
    GLFWwindow* MouseWindow;
    GLFWcursor* MouseCursors[ImGuiMouseCursor_COUNT];
    ImVec2                  LastValidMousePos;
    GLFWwindow* KeyOwnerWindows[GLFW_KEY_LAST];
    bool                    InstalledCallbacks;
    bool                    WantUpdateMonitors;

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    GLFWwindowfocusfun      PrevUserCallbackWindowFocus;
    GLFWcursorposfun        PrevUserCallbackCursorPos;
    GLFWcursorenterfun      PrevUserCallbackCursorEnter;
    GLFWmousebuttonfun      PrevUserCallbackMousebutton;
    GLFWscrollfun           PrevUserCallbackScroll;
    GLFWkeyfun              PrevUserCallbackKey;
    GLFWcharfun             PrevUserCallbackChar;
    GLFWmonitorfun          PrevUserCallbackMonitor;

    ImGui_ImplGlfw_Data_internal() { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGui_ImplGlfw_ViewportData_internal
{
    GLFWwindow* Window;
    bool        WindowOwned;
    int         IgnoreWindowPosEventFrame;
    int         IgnoreWindowSizeEventFrame;

    ImGui_ImplGlfw_ViewportData_internal() { Window = NULL; WindowOwned = false; IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1; }
    ~ImGui_ImplGlfw_ViewportData_internal() { }
};

static ImGui_ImplGlfw_Data_internal* ImGui_ImplGlfw_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplGlfw_Data_internal*)ImGui::GetIO().BackendPlatformUserData : NULL;
}

static void ImGui_ImplGlfw_CreateWindow(ImGuiViewport* viewport);

void impl_custom_imgui_platform::shutdown_custom_imgui_platform()
{
}

static void ImGui_custom_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGui_ImplGlfw_Data_internal* bd = ImGui_ImplGlfw_GetBackendData();
    ImGui_ImplGlfw_ViewportData_internal vd = *(ImGui_ImplGlfw_ViewportData_internal*)viewport->PlatformUserData;
    auto gl_command_queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
    ExecutableCommand* command = (ExecutableCommand*)new OpenGLRenderCommandAdapter([vd]() {

        glfwMakeContextCurrent(vd.Window);
        glfwSwapBuffers(vd.Window);
        });
    gl_command_queue->ExecuteCommand(command);
}

static void ImGui_custom_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGui_ImplGlfw_Data_internal* bd = ImGui_ImplGlfw_GetBackendData();
    ImGui_ImplGlfw_ViewportData_internal vd = *(ImGui_ImplGlfw_ViewportData_internal*)viewport->PlatformUserData;
    auto gl_command_queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
    ExecutableCommand* command = (ExecutableCommand*)new OpenGLRenderCommandAdapter([vd]() {
        glfwMakeContextCurrent(vd.Window);
        });
    gl_command_queue->ExecuteCommand(command);
}

static void ImGui_ImplGlfw_WindowCloseCallback(GLFWwindow* window)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
        viewport->PlatformRequestClose = true;
}

static void ImGui_ImplGlfw_WindowPosCallback(GLFWwindow* window, int, int)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
    {
        if (ImGui_ImplGlfw_ViewportData_internal* vd = (ImGui_ImplGlfw_ViewportData_internal*)viewport->PlatformUserData)
        {
            bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
            //data->IgnoreWindowPosEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestMove = true;
    }
}

static void ImGui_ImplGlfw_WindowSizeCallback(GLFWwindow* window, int, int)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
    {
        if (ImGui_ImplGlfw_ViewportData_internal* vd = (ImGui_ImplGlfw_ViewportData_internal*)viewport->PlatformUserData)
        {
            bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
            //data->IgnoreWindowSizeEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestResize = true;
    }
}


static void ImGui_custom_CreateWindow(ImGuiViewport* viewport)
{
    auto gl_command_queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
    std::shared_ptr<RenderFence> fence = std::shared_ptr<RenderFence>(Renderer::Get()->GetFence());
    ExecutableCommand* command = (ExecutableCommand*)new OpenGLRenderCommandAdapter([]() {
        glfwMakeContextCurrent(nullptr);
        });
    gl_command_queue->ExecuteCommand(command);
    gl_command_queue->Signal(fence, 1);
    fence->WaitForValue(1);
        ImGui_ImplGlfw_Data_internal* bd = ImGui_ImplGlfw_GetBackendData();
        ImGui_ImplGlfw_ViewportData_internal* vd = IM_NEW(ImGui_ImplGlfw_ViewportData_internal)();
        viewport->PlatformUserData = vd;

        // GLFW 3.2 unfortunately always set focus on glfwCreateWindow() if GLFW_VISIBLE is set, regardless of GLFW_FOCUSED
        // With GLFW 3.3, the hint GLFW_FOCUS_ON_SHOW fixes this problem
        glfwWindowHint(GLFW_VISIBLE, false);
        glfwWindowHint(GLFW_FOCUSED, false);
    #if GLFW_HAS_FOCUS_ON_SHOW
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, false);
    #endif
        glfwWindowHint(GLFW_DECORATED, (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true);
    #if GLFW_HAS_WINDOW_TOPMOST
        glfwWindowHint(GLFW_FLOATING, (viewport->Flags & ImGuiViewportFlags_TopMost) ? true : false);
    #endif
        GLFWwindow* share_window = bd->Window;
        vd->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", NULL, share_window);
        vd->WindowOwned = true;
        viewport->PlatformHandle = (void*)vd->Window;
    #ifdef _WIN32
        viewport->PlatformHandleRaw = glfwGetWin32Window(vd->Window);
    #endif
        glfwSetWindowPos(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

        // Install GLFW callbacks for secondary viewports
        glfwSetWindowFocusCallback(vd->Window, ImGui_ImplGlfw_WindowFocusCallback);
        glfwSetCursorEnterCallback(vd->Window, ImGui_ImplGlfw_CursorEnterCallback);
        glfwSetCursorPosCallback(vd->Window, ImGui_ImplGlfw_CursorPosCallback);
        glfwSetMouseButtonCallback(vd->Window, ImGui_ImplGlfw_MouseButtonCallback);
        glfwSetScrollCallback(vd->Window, ImGui_ImplGlfw_ScrollCallback);
        glfwSetKeyCallback(vd->Window, ImGui_ImplGlfw_KeyCallback);
        glfwSetCharCallback(vd->Window, ImGui_ImplGlfw_CharCallback);
        glfwSetWindowCloseCallback(vd->Window, ImGui_ImplGlfw_WindowCloseCallback);
        glfwSetWindowPosCallback(vd->Window, ImGui_ImplGlfw_WindowPosCallback);
        glfwSetWindowSizeCallback(vd->Window, ImGui_ImplGlfw_WindowSizeCallback);


}

static void ImGui_custom_DestroyWindow(ImGuiViewport* viewport)
{
    auto gl_command_queue = static_cast<OpenGLRenderCommandQueue*>(Renderer::Get()->GetCommandQueue());
    std::shared_ptr<RenderFence> fence = std::shared_ptr<RenderFence>(Renderer::Get()->GetFence());
    gl_command_queue->Signal(fence, 1);
    fence->WaitForValue(1);
    ImGui_ImplGlfw_Data_internal* bd = ImGui_ImplGlfw_GetBackendData();
    if (ImGui_ImplGlfw_ViewportData_internal* vd = (ImGui_ImplGlfw_ViewportData_internal*)viewport->PlatformUserData)
    {
        if (vd->WindowOwned)
        {
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
            HWND hwnd = (HWND)viewport->PlatformHandleRaw;
            ::RemovePropA(hwnd, "IMGUI_VIEWPORT");
#endif

            // Release any keys that were pressed in the window being destroyed and are still held down,
            // because we will not receive any release events after window is destroyed.
            for (int i = 0; i < IM_ARRAYSIZE(bd->KeyOwnerWindows); i++)
                if (bd->KeyOwnerWindows[i] == vd->Window)
                    ImGui_ImplGlfw_KeyCallback(vd->Window, i, 0, GLFW_RELEASE, 0); // Later params are only used for main viewport, on which this function is never called.

            glfwDestroyWindow(vd->Window);
        }
        vd->Window = NULL;
        IM_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = NULL;
}

void impl_custom_imgui_platform::init_custom_imgui_platform()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_RenderWindow = ImGui_custom_RenderWindow;
    platform_io.Platform_CreateWindow = ImGui_custom_CreateWindow;
    platform_io.Platform_SwapBuffers = ImGui_custom_SwapBuffers;
    platform_io.Platform_DestroyWindow = ImGui_custom_DestroyWindow;
}


#endif