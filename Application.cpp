#include "Application.h"
#include <Renderer/MeshManager.h>
#include <Renderer/TextureManager.h>
#include "Window.h"
#include <FileManager.h>
#include <ConfigManager.h>
#include <Renderer/Renderer.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/MeshComponent.h>
#include <World/Systems/BoxRenderer.h>
#include "Layer.h"
#include <World/EntityManager.h>
#include <Profiler.h>
#include <Input/Input.h>
#include <TaskSystem.h>
#include <GameStateMachine.h>
#include <GameLayer.h>
#include <stdexcept>
#include <FrameManager.h>

#ifdef EDITOR
#include <Editor/Editor.h>
#endif


Application* Application::instance = nullptr;

Application *Application::Get()
{
    return instance;
}

Window* Application::GetWindow() const
{
    return m_Window;
}

bool Application::SendEvent(Event* event)
{
#ifdef EDITOR
    if (Editor::Get()->OnEvent(event)) {
        return true;
    }
#endif
    
    GameStateMachine::Get()->OnEventState(event);
    m_GameLayer->OnEvent(event);
    return event->handled;
}

Application::~Application()
{

#ifdef EDITOR
    Editor::Shutdown();
#endif
    
    ShutdownSystems();
    
    EntityManager::Shutdown();

    Input::Shutdown();

    GameStateMachine::Shutdown();

    FrameManager::Shutdown();
    delete async_dispather;

    TextureManager::Shutdown();
    MeshManager::Shutdown();
    Renderer::Shutdown();
    delete m_Window;
    
    TaskSystem::Shutdown();
    ShutdownThread();
    m_TaskThreads.clear();
    ThreadManager::Get()->JoinedThreadUnRegister();
    m_MainThread.reset();
    ThreadManager::Shutdown();
    ConfigManager::Shutdown();
    FileManager::Shutdown();
    delete m_GameLayer;
    delete world;
}

Application::Application()
    : m_Window(nullptr), world()
{
    
}

void Application::InitInstance()
{
    //Initialize Config store
    ConfigManager::Init(FileManager::GetRelativeBinaryPath("/../config.json"));
    
    //Initialize FileManager
    FileManager::Init();

    //Init GameState
    GameStateMachine::Init();

    //ThreadManagerStartup
    ThreadManager::Init();

    async_dispather = new AsyncTaskDispatcher;

    async_dispather->Run([]() {PROFILE("AsyncThread");});

    m_GameLayer = new GameLayer();

    world = new World();

    //Claim MainThread
    m_MainThread = ThreadManager::Get()->GetThread();

    ThreadManager::Get()->JoinedThreadRegister(m_MainThread);

    //Create window and Renderer
    m_Window = Window::CreateWindow();
    Renderer::Create();

    //Window and renderer Pre-initialization phase
    m_Window->PreInit();
    Renderer::Get()->PreInit();

    PreInitializeSystems();

    //TaskSystem Startup
    TaskSystem::Initialize();
    TaskSystem::Get()->Run(&InitThread,&ShutdownThread);

    //Initialize MainThread as JoinedThread
    InitThread();

    //Window and renderer Initialization phase
    m_Window->Init();
    Renderer::Get()->Init();
    MeshManager::Init();
    TextureManager::Init();

    FrameManager::Initialize();

    Input::Init();

    EntityManager::Initialize();

    InitializeSystems();

#ifdef EDITOR
    Editor::Init();
#endif


}

void Application::PreInitializeSystems()
{
    ScriptSystemManager::Initialize();
}

void Application::InitializeSystems()
{
    
}

void Application::ShutdownSystems()
{
    ScriptSystemManager::Shutdown();
    GetWorld().GetRegistry().clear<MeshComponent>();
    GetWorld().GetRegistry().clear<LoadingMeshComponent>();
    Delete_Render_Box_data();
}

void Application::Exit()
{
    if (m_running) {
        m_running = false;
    }
    else {
        throw std::runtime_error("Application exited at initialization");
    }
}

void Application::Run()
{
    m_running = true;
    last_time_point = std::chrono::high_resolution_clock::now();
    
    //THE Game Loop
    while (m_running)
    {
        PROFILE("Update");
        Update();
    }

    OnGameStop();
}

void Application::SetInitialGameState(std::shared_ptr<GameState> state)
{
    if (!m_running) {
        GameStateMachine::Get()->ChangeState(state);
    }
    else {
        throw std::runtime_error("Cannot set InitialGameState at runtime.");
    }
}

void Application::Update()
{
#ifdef EDITOR
    Editor::Get()->ViewportBegin();
#endif
    
    //Synchronize with RenderThread
    FrameManager::Get()->StartFrame();

    //Calculate delta_time
    std::chrono::nanoseconds time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - last_time_point);
    float delta_time = (float)time_diff.count() / 1000000;
    last_time_point = std::chrono::high_resolution_clock::now();
    if (delta_time == 0) {
        delta_time = 1;
    }

    m_GameLayer->LoadSystem();
    
    //Update current GameState
    GameStateMachine::Get()->UpdateNextState();
   
    //Update GameState and Layers
    m_GameLayer->PreUpdate(delta_time);
   
    //Poll Events and execute event and input handlers
    m_Window->PollEvents();

   

    PROFILE("Layer Update");
    GameStateMachine::Get()->UpdateState(delta_time);
    m_GameLayer->OnUpdate(delta_time);

#ifdef EDITOR
    Editor::Get()->ViewportEnd();
    Editor::Get()->Run();
    Editor::Get()->Render();
#endif
    //Present / Swap buffers
    PROFILE("SwapBuffers");
    Renderer::Get()->GetCommandQueue()->Present();

    //Flush TaskSystem MemoryPool deallocations
    TaskSystem::Get()->FlushDeallocations();
    async_dispather->FlushDeallocations();
}

void Application::InitThread()
{
    if (ThreadManager::IsValidThreadContext()) {
        ScriptSystemManager::Get()->InitThread();
    }
    else {
        throw std::runtime_error("Thread Initialization failed");
    }
}

void Application::ShutdownThread()
{
    if (ThreadManager::IsValidThreadContext()) {

    }
    else {
        throw std::runtime_error("Thread Initialization failed");
    }
}

void Application::Init()
{
    if (!instance) {
        instance = new Application();
        instance->InitInstance();
    }
}

void Application::ShutDown()
{
    if (instance) {
        delete instance;
    }
}

void Application::OnGameStop()
{

}

