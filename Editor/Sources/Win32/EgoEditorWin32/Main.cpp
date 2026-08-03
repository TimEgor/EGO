#include "EgoEditor/EditorApplication.h"

#include <Windows.h>

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
    ego::editor::EditorApplication application;
    if (!application.init(_hInstance, __argc, __argv))
    {
        return ego::editor::EditorApplication::InitializationFailedExitCode;
    }

    return application.run();
}
