#include "OSApi.h"
#include <platform/Linux/LinuxOSApi.h>
#include <platform/Windows/WindowsOSApi.h>

OSApi* OSApi::CreateOSApi()
{
#ifdef WINDOWS
    return new WindowsOSApi();
#elif defined LINUX
    return new LinuxOSApi();
#else
    static_assert(false, "Only Windows and Linux platforms are supported");
#endif
}
