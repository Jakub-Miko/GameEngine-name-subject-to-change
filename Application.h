#pragma once
#include <vector>
#include <memory>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <cstddef>
#include <GameLayer.h>
#include <chrono>
#include <Events/Event.h>
#include <ThreadManager.h>
#include <Renderer/RenderFence.h>

class Window;
class Renderer;
class Layer;
class GameState;
class Application {
private:
    static Application* instance;
    Window* m_Window;
    int latency_frames = 0;
    std::vector<std::shared_ptr<ThreadObject>> m_TaskThreads;
    std::shared_ptr<ThreadObject> m_MainThread;
    bool m_running = false;
    std::shared_ptr<RenderFence> m_Sync_Fence;
    uint32_t frame_count = 0;

    friend class GameState;
    GameLayer* m_GameLayer = nullptr;
 
public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    static Application* Get();

    Window* GetWindow() const;

    bool SendEvent(Event* event);

    void Exit();

    void SetInitialGameState(std::shared_ptr<GameState> state);

    void Run();

    void Update();

    static void Init();

    static void ShutDown();

    static World& GetWorld() { return instance->m_GameLayer->GetWorld(); };


private:
    std::chrono::high_resolution_clock::time_point last_time_point;

    void InitInstance();

    void InitializeSystems();
    void ShutdownSystems();

    ~Application();
    Application();

    void OnGameStop();
};
