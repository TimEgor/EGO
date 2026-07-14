#include "StandaloneApplication.h"

#include <Windows.h>

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
    ego::demo::standalone::StandaloneApplication application;
    if (!application.init(_hInstance, __argc, __argv))
    {
        return ego::demo::standalone::StandaloneApplication::InitializationFailedExitCode;
    }

    return application.run();
}
