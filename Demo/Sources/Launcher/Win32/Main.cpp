#include "LauncherApplication.h"

#include "EgoApplication/Application.h"

#include <Windows.h>

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
    ego::demo::launcher::LauncherApplicationPointer application = new ego::demo::launcher::LauncherApplication();
    if (!application)
    {
        return ego::demo::launcher::LauncherApplication::ApplicationCoreInitializationFailedExitCode;
    }

    if (!ego::application::ApplicationCore::GetInstance().init(application))
    {
        return ego::demo::launcher::LauncherApplication::ApplicationCoreInitializationFailedExitCode;
    }

    const int result = application->run(_hInstance, __argc, __argv);
    ego::application::ApplicationCore::GetInstance().release();

    return result;
}
