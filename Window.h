#pragma once
#include <string>

struct WindowProperties {
    int resolution_x = 1280, resolution_y = 720;
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

    virtual ~Window() {};
public:

    WindowProperties m_Properties;

    static Window* CreateWindow(const WindowProperties& props = WindowProperties());
};
