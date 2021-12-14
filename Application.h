#pragma once
#include <vector>
#include <memory>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <cstddef>
#include <chrono>
#include <Events/Event.h>
#include <Renderer/RenderFence.h>

class Window;
class Renderer;
class Application {
private:
    static Application* instance;
    Window* m_Window;
    int latency_frames = 0;
    bool m_running = false;
    std::shared_ptr<RenderFence> m_Sync_Fence;
    uint32_t frame_count = 0;

 
public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    static Application* Get();

    Window* GetWindow() const;

    bool SendEvent(Event* event);

    void Exit();

    void Run();

    void Update();

    static void InitThread();

    static void ShutdownThread();

    static void Init();

    static void ShutDown();


private:
    std::chrono::high_resolution_clock::time_point last_time_point;

    void InitInstance();

    void PreInitializeSystems();

    void InitializeSystems();
    void ShutdownSystems();

    ~Application();
    Application();

    void OnGameStop();
};
