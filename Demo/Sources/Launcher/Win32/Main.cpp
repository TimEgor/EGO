#include "LauncherApplication.h"

#include <Windows.h>

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
    ego::demo::launcher::LauncherApplication application;
    return application.run(_hInstance, __argc, __argv);
}
