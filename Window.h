#pragma once
#include <string>

struct WindowProperties {
    int resolution_x = 800, resolution_y = 600;
    std::string name = "Hello World";
};

class Window {
public: 
public:
    Window(const WindowProperties& props);
    
    virtual void PreInit() = 0;

    virtual void Init() = 0;

    virtual void PollEvents() = 0;

    virtual ~Window() {};
public:

    WindowProperties m_Properties;

    static Window* CreateWindow(const WindowProperties& props = WindowProperties());
};
