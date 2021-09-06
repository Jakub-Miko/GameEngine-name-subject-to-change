#pragma once
#include <string>

class Window {
public: 
    struct Properties {
        int resolution_x = 800, resolution_y = 600;
        std::string name = "Hello World";
    };
public:
    Window(const Properties& props);
    
    virtual void PreInit() = 0;

    virtual void Init() = 0;

    virtual void PollEvents() = 0;

    virtual ~Window() {};
public:

    Properties m_Properties;

    static Window* CreateWindow(const Properties& props = Properties());
};
