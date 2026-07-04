#include "EgoApplication/Application.h"

#include "LauncherApplication.h"

#include <Windows.h>

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
    ego::demo::launcher::LauncherApplicationPointer application = new ego::demo::launcher::LauncherApplication();
    if (!application)
    {
        return ego::demo::launcher::LauncherApplication::ApplicationInitializationFailedExitCode;
    }

    const int result = application->run(_hInstance, __argc, __argv);

    return result;
}
