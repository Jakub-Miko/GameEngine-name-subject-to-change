#pragma once
#include <vector>
#include <memory>
#include <Renderer/Renderer.h>
#include <Utility/Profiler.h>

class Window;
class Renderer;
class Layer;

class Application {
private:
    Window* m_Window;
    std::shared_ptr<Renderer> m_Renderer;
    std::vector<Layer*> m_Layers;
    bool m_running = false;

public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    static Application* Get();

    std::shared_ptr<Renderer> GetRenderer() const { return m_Renderer; };

    void PushLayer(Layer* layer);

    void Init();

    void Exit();

    void Run();

    void Update();

private:
    Application();

    void OnGameStop();
};