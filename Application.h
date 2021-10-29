#pragma once
#include <vector>
#include <memory>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <ThreadManager.h>

class Window;
class Renderer;
class Layer;

class Application {
private:
    static Application* instance;
    Window* m_Window;
    std::vector<Layer*> m_Layers;
    std::vector<std::shared_ptr<ThreadObject>> m_TaskThreads;
    std::shared_ptr<ThreadObject> m_MainThread;
    bool m_running = false;

public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    static Application* Get();

    Window* GetWindow() const;

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