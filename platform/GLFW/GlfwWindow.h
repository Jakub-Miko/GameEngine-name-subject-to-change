#pragma once
#include <Window.h>

struct GLFWwindow;

class GlfwWindow : public Window {
public:
    virtual void Init() override;
    virtual void PreInit() override;

    virtual void PollEvents() override;

    virtual ~GlfwWindow();
public:
    GLFWwindow* m_Window;
};