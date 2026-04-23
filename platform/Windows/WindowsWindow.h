#pragma once
#include <Window.h>

struct HWND__;
typedef HWND__* HWND;

class WindowsWindow : public Window {
public:
    WindowsWindow(const WindowProperties& props);

    virtual void PollEvents() override;

    virtual void SwapBuffers() override;

    virtual glm::ivec2 GetFramebufferResolution() override;

    virtual bool IsMinimized() override;

    virtual ~WindowsWindow();
public:
    HWND m_Window;
};