#pragma once
#include <string>
#include <vector>

struct WindowProperties {
    int resolution_x = -1, resolution_y = -1;
    std::string name = "GameEngine NSTC";
};

class Window {
public:
    Window(const WindowProperties& props);
    
    virtual void PreInit() = 0;

    virtual void Init() = 0;

    virtual void PollEvents() = 0;

    const WindowProperties& GetProperties() const {
        return m_Properties;
    }

    virtual void SwapBuffers() = 0;

    virtual void DisableCursor() = 0;
    virtual void EnableCursor() = 0;

#ifdef EDITOR

    virtual void AdjustWidowToDisabledEditor() = 0;

    virtual void AdjustWidowToEnabledEditor() = 0;


#endif

    virtual void RegistorDragAndDropCallback(void(*callback)(int count, std::vector<std::string> paths)) = 0;

    virtual ~Window() {};
public:

    WindowProperties m_Properties;

    static Window* CreateWindow(const WindowProperties& props = WindowProperties());
};
