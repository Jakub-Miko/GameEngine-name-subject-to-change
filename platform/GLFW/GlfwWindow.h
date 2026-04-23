#pragma once
#include <Window.h>
#include <Renderer/RenderSurface.h>

struct GLFWwindow;
class OpenGLRenderContext;

class GlfwWindow : public Window, public std::enable_shared_from_this<GlfwWindow> {
public:
    friend class impl_custom_imgui_platform;
    GlfwWindow(const WindowProperties& props);
    
    static void Init();
    static void Shutdown();

    virtual void PollEvents() override;

    virtual void SwapBuffers() override;

    virtual void RegistorDragAndDropCallback(void(*callback)(int count, std::vector<std::string> paths)) override;

    virtual void DisableCursor() override;
    virtual void EnableCursor() override;

    virtual glm::ivec2 GetFramebufferResolution() override;

    virtual bool IsMinimized() override;

#ifdef EDITOR

    virtual void AdjustWidowToDisabledEditor() override;

    virtual void AdjustWidowToEnabledEditor() override;
    
    
#endif

    void InitSurface();

    virtual std::shared_ptr<RenderSurface> GetRenderSurface() override {
        if (!window_render_surface) {
            InitSurface();
        }
        return window_render_surface;
    }

    
    virtual ~GlfwWindow();
    

    GLFWwindow* GetHandle() const { return m_Window; }
private:

    static void DropCallback(GLFWwindow* window, int count, const char** paths);

    static void DefaultDropCallback(int count, std::vector<std::string> paths);

    GLFWwindow* m_Window;
    void(*drop_callback)(int count, std::vector<std::string>paths);
    std::shared_ptr<RenderSurface> window_render_surface;
};