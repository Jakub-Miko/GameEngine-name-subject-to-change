#include "Application.h"
#include "Window.h"
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <stdexcept>


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
    //GameStateMachine::Get()->OnEventState(event);
    return event->handled;
}

Application::~Application()
{
    //ShutdownSystems();
    
    //EntityManager::Shutdown();

    //Input::Shutdown();
    Renderer::Shutdown();
    delete m_Window;
    

    //TaskSystem::Shutdown();
    //ShutdownThread();
  /*  m_TaskThreads.clear();*/
    //ThreadManager::Get()->JoinedThreadUnRegister();
   /* m_MainThread.reset();
    ThreadManager::Shutdown();*/
    //ConfigManager::Shutdown();
    //FileManager::Shutdown();
    //GameStateMachine::Shutdown();
    //delete m_GameLayer;
}

Application::Application()
    : m_Window(nullptr)
{
    
}

void Application::InitInstance()
{
    ////Initialize FileManager
    //FileManager::Init();

    ////Initialize Config store
    //ConfigManager::Init(FileManager::Get()->GetRelativeFilepath("config.json"));

    ////Init GameState
    //GameStateMachine::Init();

    //ThreadManagerStartup
    //ThreadManager::Init();

    //m_GameLayer = new GameLayer();

    ////Claim MainThread
    //m_MainThread = ThreadManager::Get()->GetThread();

    //ThreadManager::Get()->JoinedThreadRegister(m_MainThread);

    //Create window and Renderer
    m_Window = Window::CreateWindow();
    Renderer::Create();

    //Window and renderer Pre-initialization phase
    m_Window->PreInit();
    Renderer::Get()->PreInit();

    //PreInitializeSystems();

    ////TaskSystem Startup
    //TaskSystem::Initialize();
    //TaskSystem::Get()->Run(&InitThread,&ShutdownThread);

    ////Initialize MainThread as JoinedThread
    //InitThread();

    //Window and renderer Initialization phase
    m_Window->Init();
    Renderer::Get()->Init();

    m_Sync_Fence.reset(Renderer::Get()->GetFence());

    latency_frames = 1;
    frame_count = latency_frames;

    //Input::Init();

    //EntityManager::Initialize();

    //InitializeSystems();
}

void Application::PreInitializeSystems()
{
    //ScriptSystemManager::Initialize();
}

void Application::InitializeSystems()
{
    
}

void Application::ShutdownSystems()
{
    //ScriptSystemManager::Shutdown();
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
    while (m_running)
    {
        PROFILE("Update");
        Update();
    }
    OnGameStop();
}


void Application::Update()
{
    //Synchronize with RenderThread
    Renderer::Get()->GetCommandQueue()->Signal(m_Sync_Fence, frame_count);
    m_Sync_Fence->WaitForValue(frame_count - latency_frames);
    ++frame_count;

    ////Calculate delta_time
    //std::chrono::nanoseconds time_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - last_time_point);
    //float delta_time = (double)time_diff.count() / 1000000;
    //last_time_point = std::chrono::high_resolution_clock::now();
    //if (delta_time == 0) {
    //    delta_time = 1;
    //}

    //GameStateMachine::Get()->UpdateNextState();

    //Poll Event and execute event and input handlers
    m_Window->PollEvents();

    //Update GameState and Layers
    //PROFILE("Layer Update");
    //GameStateMachine::Get()->UpdateState(delta_time);
    auto list = Renderer::Get()->GetRenderCommandList();
    list->DrawSquare({ 0.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });

    Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
    //Present / Swap buffers
    PROFILE("SwapBuffers");
    Renderer::Get()->GetCommandQueue()->Present();

    ////Flush TaskSystem MemoryPool deallocations
    //TaskSystem::Get()->FlushDeallocations();
}

void Application::InitThread()
{

}

void Application::ShutdownThread()
{

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

