#include "Application.h"
#include "Window.h"
#include <Renderer/Renderer.h>
#include "Layer.h"
#include <Utility/Profiler.h>

Application *Application::Get()
{
    static Application* Instance = new Application();
    return Instance;
}

Application::Application()
    :m_Renderer(nullptr), m_Window(nullptr)
{

}

void Application::Init()
{
    m_Window = Window::CreateWindow();
    m_Renderer = std::shared_ptr<Renderer>(Renderer::CreateRenderer());

    m_Window->PreInit();
    m_Renderer->PreInit();

    m_Window->Init();
    m_Renderer->Init();
}

void Application::Exit()
{
    m_running = false;
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
        m_Renderer->Render();
    }
    PROFILE("SwapBuffers");
    m_Window->SwapBuffers();
}

void Application::OnGameStop()
{
    delete m_Window;
    for (auto Layer : m_Layers)
    {
        if (Layer)
        {
            delete Layer;
        }
    }
}

void Application::PushLayer(Layer *layer)
{
    m_Layers.push_back(layer);
}
