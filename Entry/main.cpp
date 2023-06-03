#include "Application.h"
#include "GameLayer.h"
#include <Profiler.h>
#include <iostream>
#include <States/SandboxState.h>
#include <Renderer/RenderDescriptorHeapBlock.h>

#ifdef WIN32
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

int main() {
    {
#ifdef WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
        BEGIN_PROFILING("Profile", "C:/Users/mainm/Desktop/GameEngine/GameEngine-name-subject-to-change/Profile_Result1.json");
        try {
            Application::Init();
            Application::Get()->SetInitialGameState(std::make_shared<SandboxState>());
            Application::Get()->Run();
            Application::ShutDown();
        }
        catch(std::runtime_error& error) {
            std::cout << error.what() << "\n";
        }

        END_PROFILING();

    }
}