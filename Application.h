#pragma once
#include <vector>
#include <memory>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <cstddef>
#include <GameLayer.h>
#include <World/World.h>
#include <chrono>
#include <Events/Event.h>
#include <mutex>
#include <AsyncTaskDispatcher.h>
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
    std::vector<std::shared_ptr<ThreadObject>> m_TaskThreads;
    std::shared_ptr<ThreadObject> m_MainThread;
    bool m_running = false;
    AsyncTaskDispatcher* async_dispather = nullptr;
    friend class GameState;
    GameLayer* m_GameLayer = nullptr;
    World* world = nullptr;

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

    static void InitThread();

    static void ShutdownThread();

    static void Init();

    static void ShutDown();

    static World& GetWorld() { return *instance->world; };

    static AsyncTaskDispatcher* GetAsyncDispather() { return instance->async_dispather; }

    static std::mutex& GetDebugMutex() {
        static std::mutex mutex;
        return mutex;
    }

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
