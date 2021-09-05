#pragma once

class Window {
public:
    virtual void PreInit() = 0;

    virtual void Init() = 0;

    virtual void PollEvents() = 0;

    virtual ~Window() {};
public:
    
    static Window* CreateWindow();
};
