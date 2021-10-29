#include "Application.h"
#include "Window.h"

#include <Renderer/Renderer.h>
#include "Layer.h"
#include <Profiler.h>
#include <TaskSystem.h>
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

Application::~Application()
{
    delete m_Window;
    Renderer::Shutdown();
    for (auto Layer : m_Layers)
    {
        if (Layer)
        {
            delete Layer;
        }
    }
    m_Layers.clear();
    TaskSystem::Shutdown();
    m_TaskThreads.clear();
    m_MainThread.reset();
    ThreadManager::Shutdown();
}

Application::Application()
    : m_Window(nullptr)
{
    InitInstance();
}

void Application::InitInstance()
{
    //ThreadManagerStartup
    ThreadManager::Init();
    
    //Claim MainThread
    m_MainThread = ThreadManager::GetThreadManager()->GetThread();
    
    //Create window and Renderer
    m_Window = Window::CreateWindow();
    Renderer::Create();

    //Window and renderer Pre-initialization phase
    m_Window->PreInit();
    Renderer::Get()->PreInit();
    
    //TaskSystem Startup
    int task_threads = ThreadManager::GetThreadManager()->GetAvailableThreadCount();
    for (int i = 0; i < task_threads; i++) {
        m_TaskThreads.push_back(ThreadManager::GetThreadManager()->GetThread());
    }
    TaskSystem::Initialize(TaskSystemProps{ task_threads });
    TaskSystem::Get()->Run();

    //Window and renderer Initialization phase
    m_Window->Init();
    Renderer::Get()->Init();
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
    while (m_running)
    {
        PROFILE("Update");
        Update();
    }
    OnGameStop();
}

void Application::Update()
{
    m_Window->PollEvents();
    for (auto layer : m_Layers) 
    {
        PROFILE("Layer Update");
        layer->OnUpdate();
    }
    TaskSystem::Get()->FlushDeallocations();
    PROFILE("SwapBuffers");
    m_Window->SwapBuffers();
}

void Application::Init()
{
    if (!instance) {
        instance = new Application();
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

void Application::PushLayer(Layer *layer)
{
    m_Layers.push_back(layer);
}
