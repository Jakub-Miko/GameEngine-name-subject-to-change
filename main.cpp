#include "Application.h"
#include "GameLayer.h"
#include <Utility/Profiler.h>
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

int main() {
    {
        BEGIN_PROFILING("Profile", "C:/Users/mainm/Desktop/GameEngine/PseudoCode/Profile_Result.json");
        
        Application* app = Application::Get();
        app->PushLayer(new GameLayer());
        app->Init();
        app->Run(); 
        delete app;
        END_PROFILING();
    }
    //_CrtDumpMemoryLeaks();
}