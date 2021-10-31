#include "Application.h"
#include "GameLayer.h"
#include <Profiler.h>

#ifdef WIN32
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

int main() {
    {
        BEGIN_PROFILING("Profile", "C:/Users/mainm/Desktop/GameEngine/PseudoCode/Profile_Result1.json");
        Application::Init();
        Application::Get()->PushLayer(new GameLayer());
        Application::Get()->Run();
        Application::ShutDown();
        END_PROFILING();
    }
#ifdef WIN32
    _CrtDumpMemoryLeaks();
#endif
}