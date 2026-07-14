#include "WinPIXProfiler.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <Windows.h>
#include <WinPixEventRuntime/pix3.h>

void ego::winpix::WinPIXProfiler::beginEvent(const char* _titleName, const char* _contextName)
{
    const char* titleName = _titleName ? _titleName : "";
    if (_contextName && _contextName[0] != '\0')
    {
        PIXBeginEvent(PIX_COLOR_DEFAULT, "%s: %s", titleName, _contextName);
        return;
    }

    PIXBeginEvent(PIX_COLOR_DEFAULT, "%s", titleName);
}

void ego::winpix::WinPIXProfiler::endEvent()
{
    PIXEndEvent();
}
