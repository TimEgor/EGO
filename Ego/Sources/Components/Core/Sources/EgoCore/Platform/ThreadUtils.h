#pragma once

#include "EgoCore/String/StringConverter.h"
#include "PlatformMacros.h"

namespace ego::threadUtils
{
    inline void SetThreadName(void* threadHandle, const char* name)
    {
#ifdef EGO_WIN_PLATFORM
        SetThreadDescription(threadHandle, ConvertStringToWString(name).c_str());
#endif
    }

    inline void SetThreadName(const char* name)
    {
#ifdef EGO_WIN_PLATFORM
        SetThreadName(GetCurrentThread(), name);
#endif
    }
} // namespace ego::threadUtils
