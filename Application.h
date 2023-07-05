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
#include <Events/SubjectObserver.h>
#include <mutex>
#include <AsyncTaskDispatcher.h>
#include <ThreadManager.h>
#include <Renderer/RenderFence.h>
#include <OSApi.h>


#include <Core/Defines.h>
LIBEXP class TestModular {
public:
    RUNTIME_TAG("TestModular");
    virtual TestModular* clone() = 0;
    virtual int GetNumber1() = 0;
    virtual int GetNumber2() = 0;
    virtual int GetNumber3() = 0;

};

class Window;
class Renderer;
class Layer;
class GameState;
class Application {
private:
    static Application* instance;
    Window* m_Window;
    OSApi* os_api;
    std::vector<std::shared_ptr<ThreadObject>> m_TaskThreads;
    std::shared_ptr<ThreadObject> m_MainThread;
    bool m_running = false;
    float delta_time = 0;
    AsyncTaskDispatcher* async_dispather = nullptr;
    friend class GameState;
    GameLayer* m_GameLayer = nullptr;
    World* world = nullptr;
    std::unordered_map<RuntimeTagIdType, EventSubject> event_subjects;
    std::mutex event_subjects_mutex;

public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    static Application* Get();

    Window* GetWindow() const;

    OSApi* GetOsApi() const {
        return os_api;
    }

    template<typename EventType>
    void SendObservedEvent(EventType* e) {
        //TODO: this can be synchronized more granularily
        std::unique_lock<std::mutex> lock(event_subjects_mutex);
        auto fnd = event_subjects.find(e->GetType());
        if (fnd == event_subjects.end()) {
            fnd = event_subjects.emplace(std::piecewise_construct,
                std::forward_as_tuple(e->GetType()), std::forward_as_tuple()).first;
        }
        EventSubject& sub = fnd->second;
        lock.unlock();
        sub.Notify((Event*)e);
    }

    template<typename EventType>
    EventSubject& GetEventSubject() {
        //TODO: this can be synchronized more granularily
        std::unique_lock<std::mutex> lock(event_subjects_mutex);
        auto fnd = event_subjects.find(RuntimeTag<EventType>::GetId());
        if (fnd == event_subjects.end()) {
            fnd = event_subjects.emplace(std::piecewise_construct,
                std::forward_as_tuple(RuntimeTag<EventType>::GetId()), std::forward_as_tuple()).first;
        }
        return fnd->second;

    }

    template<typename EventType>
    void RegisterObserver(EventObserverBase* observer) {
        //TODO: this can be synchronized more granularily
        std::unique_lock<std::mutex> lock(event_subjects_mutex);
        auto fnd = event_subjects.find(RuntimeTag<EventType>::GetId());
        if (fnd == event_subjects.end()) {
            fnd = event_subjects.emplace(std::piecewise_construct,
                std::forward_as_tuple(RuntimeTag<EventType>::GetId()), std::forward_as_tuple()).first;
        }
        EventSubject& sub = fnd->second;
        lock.unlock();
        sub.Subscribe(observer);
    }

    bool SendEvent(Event* event);

    void Exit();

    void SetInitialGameState(std::shared_ptr<GameState> state);

    void Run();

    void Update();

    static void InitThread();

    static void ShutdownThread();

    static void Init();

    static void ShutDown();

    static float GetDeltaTime() {
        return instance->delta_time;
    }

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
