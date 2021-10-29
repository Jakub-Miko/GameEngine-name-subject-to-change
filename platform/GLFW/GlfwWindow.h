#pragma once
#include <Window.h>

struct GLFWwindow;
class OpenGLRenderContext;

class GlfwWindow : public Window {
public:
    GlfwWindow(const WindowProperties& props);
    
    virtual void Init() override;
    virtual void PreInit() override;

    virtual void PollEvents() override;

    virtual void SwapBuffers() override;

    virtual ~GlfwWindow();

    GLFWwindow* GetHandle() const { return m_Window; }
private:
    GLFWwindow* m_Window;
};