#include "LauncherApplication.h"

#include <Windows.h>

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
    ego::demo::launcher::LauncherApplication application;
    if (!application.init(_hInstance, __argc, __argv))
    {
        return ego::demo::launcher::LauncherApplication::InitializationFailedExitCode;
    }

    return application.run();
}
