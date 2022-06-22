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

    virtual void RegistorDragAndDropCallback(void(*callback)(int count, std::vector<std::string> paths)) override;

    virtual ~GlfwWindow();

    GLFWwindow* GetHandle() const { return m_Window; }
private:

    static void DropCallback(GLFWwindow* window, int count, const char** paths);

    static void DefaultDropCallback(int count, std::vector<std::string> paths);

    GLFWwindow* m_Window;
    void(*drop_callback)(int count, std::vector<std::string>paths);
};