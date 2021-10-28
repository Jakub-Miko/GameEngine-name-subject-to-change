#pragma once
#include <vector>
#include <memory>
#include <Renderer/Renderer.h>
#include <Profiler.h>

class Window;
class Renderer;
class Layer;

class Application {
private:
    static Application* instance;
    Window* m_Window;
    std::vector<Layer*> m_Layers;
    bool m_running = false;

public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    static Application* Get();

    void PushLayer(Layer* layer);

    void Exit();

    void Run();

    void Update();

    static void Init();

    static void ShutDown();

private:


    void InitInstance();

    ~Application();
    Application();

    void OnGameStop();
};