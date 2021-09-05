#include "Application.h"
#include "GameLayer.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main() {
    {
        Application* app = Application::Get();
        app->PushLayer(new GameLayer());
        app->Init();
        app->Run(); 
        delete app;
    }
    _CrtDumpMemoryLeaks();
}