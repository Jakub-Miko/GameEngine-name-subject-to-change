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

/**
 * @brief Main application singleton, which encompasses the lifetime of the entire application and handles its runtime management
 * 
 * Handles initializationm, Update, Event distribution and Shutdown of the application,
 * as well as providing access point for application globals state not contained in other Manager-type singletons
*/
class Application {
private:
    static Application* instance; ///< Singleton instance.
    Window* m_Window; ///< Main application window. Handles communication with the OS.
    OSApi* os_api; ///< Abstraction over OS specific operations, which are not handled by the window manager.
    std::vector<std::shared_ptr<ThreadObject>> m_TaskThreads; ///< Threads claimed from ThreadManager to be used by the TaskSystem.
    std::shared_ptr<ThreadObject> m_MainThread; ///< Thread claimed from ThreadManager to be the Main thread.
    bool m_running = false; ///< Varaible used to stop the GameLoop.
    float delta_time = 0; ///< Time elapsed between frames in milliseconds.
    AsyncTaskDispatcher* async_dispather = nullptr; ///< AsyncTaskDispatcher to handle asynchronous tasks.
    friend class GameState;
    GameLayer* m_GameLayer = nullptr; ///< Main Layer that handles game engine operations.
    World* world = nullptr; ///< Main world insatnce (currently the only one)
    std::unordered_map<RuntimeTagIdType, EventSubject> event_subjects; ///< Subjects for distributing events to observers across the entire application.
    std::mutex event_subjects_mutex; ///< Mutex for Application::event_subjects

public:

    Application(const Application& ref) = delete;
    Application(Application&& ref) = delete;
    Application& operator=(const Application& ref) = delete;
    Application& operator=(Application&& ref) = delete;

    /**
     * @brief Gets the Application singleton instance.
    */
    static Application* Get();

    /**
     * @brief Gets the current window.
    */
    Window* GetWindow() const;

    /**
     * @brief Gets the OS api abstraction.
    */
    OSApi* GetOsApi() const {
        return os_api;
    }

    /**
     * @brief Sends an observed event of type EventType to all its observers registered to the EventType Subject
     * @tparam EventType Type of Event, and an Observable subject.
     * @param e instance of EventType
    */
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

    /**
     * @brief Return the Event subject for an EventType
    */
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

    /**
     * @brief Registers an Observer to the subject of EventType
     * @tparam EventType event type to observe
     * @param observer observer to register
    */
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

    /**
     * @brief Distributes the Event hierarchically without using observers.
     * @param event event to distribute
     * @return whether the event was marked as proccessed (can have different implications for different events)
    */
    bool SendEvent(Event* event);

    /**
     * @brief Exits the GameLoop and start the Engine Shutdown phase.
    */
    void Exit();

    /**
     * @brief Used before startup to set the initial GameState.
     * @param state initial state
    */
    void SetInitialGameState(std::shared_ptr<GameState> state);

    /**
     * @brief Starts the Game loop
    */
    void Run();

    /**
     * @brief Iteration of the Game loop
    */
    void Update();

    /**
     * @brief Initialization task that runs for every thread, which can execute scripts and send task to async dispatcher.
    */
    static void InitThread();

    /**
     * @brief Shutdown task that runs for every thread, which can execute scripts and send task to async dispatcher.
    */
    static void ShutdownThread();

    /**
     * @brief Initialization function of the engine responsible for majority of the engines setup.
    */
    static void Init();

    /**
     * @brief ShutDown function of the engine responsible for majority of the engines shutdown. Runs after gameloop exits.
    */
    static void ShutDown();

    /**
     * @brief Get delta time in milliseconds
    */
    static float GetDeltaTime() {
        return instance->delta_time;
    }

    /**
     * @brief Get the main World instance. (Static version, only for convenience)
    */
    static World& GetWorld() { return *instance->world; };

    /**
     * @brief Get the AsyncTaskDispatcher instance. (Static version, only for convenience)
    */
    static AsyncTaskDispatcher* GetAsyncDispather() { return instance->async_dispather; }

    /**
     * @brief Get mutex for debugging in multithreaded contexts, i.e. for logging (not currently used, may be removed later)
    */
    static std::mutex& GetDebugMutex() {
        static std::mutex mutex;
        return mutex;
    }

private:
    std::chrono::high_resolution_clock::time_point last_time_point; ///< Last time frame measured when calculating the last delta time. (used for calculating ther current deltatime)
  
    /**
     * @brief called by Application::Init, this is the actual initialization method. 
    */
    void InitInstance();

    /**
     * @brief Pre-init phase, which systems use to be initialized ahead of certain other systems.
     * 
     * Certain systems claims all available resources left, i.e. the TaskSystems claims all threads, so system which require such resources need to claim them beforehand.
    */
    void PreInitializeSystems();

    /**
     * @brief Like Application::PreInitializeSystems, but runs after initialization.
    */
    void InitializeSystems();
    
    /**
     * @brief Shutdown phase for the systems.
    */
    void ShutdownSystems();

    ~Application();
    Application();

    /**
     * @brief Runs after Gameloop exits, but before shutdown begins.
    */
    void OnGameStop();
};
