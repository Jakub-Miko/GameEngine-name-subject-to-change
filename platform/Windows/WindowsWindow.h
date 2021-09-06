#pragma once
#include <Window.h>

struct HWND__;
typedef HWND__* HWND;

class WindowsWindow : public Window {
public:
    WindowsWindow(const WindowProperties& props);

    virtual void Init() override;
    virtual void PreInit() override;

    virtual void PollEvents() override;

    virtual ~WindowsWindow();
public:
    HWND m_Window;
};