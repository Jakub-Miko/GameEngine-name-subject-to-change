#include "Application.h"
#include <Profiler.h>



int main() {
        Application::Init();
        Application::Get()->Run();
        Application::ShutDown();
}