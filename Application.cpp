#include "Application.h"
#include "Window.h"
#include <Renderer/Renderer.h>
#include "Layer.h"
#include <Profiler.h>
#include <TaskSystem.h>
#include <stdexcept>

Application *Application::Get()
{
    static Application* Instance = new Application;
    return Instance;
}

Application::Application()
    : m_Window(nullptr)
{

}

void Application::Init()
{
    TaskSystem::Initialize(TaskSystemProps{ 0 });
    TaskSystem::Get()->Run();

    m_Window = Window::CreateWindow();

    m_Window->PreInit();
    Renderer::Get()->PreInit();

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
    {
        PROFILE("Render");
        Renderer::Get()->Render();
    }
    TaskSystem::Get()->FlushDeallocations();
    PROFILE("SwapBuffers");
    m_Window->SwapBuffers();
}

void Application::OnGameStop()
{
    delete m_Window;
    Renderer::Get()->Destroy();
    for (auto Layer : m_Layers)
    {
        if (Layer)
        {
            delete Layer;
        }
    }
    m_Layers.clear();
    TaskSystem::Shutdown();
}

void Application::PushLayer(Layer *layer)
{
    m_Layers.push_back(layer);
}
