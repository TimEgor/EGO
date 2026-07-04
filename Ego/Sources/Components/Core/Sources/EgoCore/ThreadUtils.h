#pragma once

#include "String/StringConverter.h"

#if defined(WIN32) || defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#endif

namespace ego::threadUtils
{
    inline void SetThreadName(void* threadHandle, const char* name)
    {
#if defined(WIN32) || defined(_WIN32)
        SetThreadDescription(threadHandle, ConvertStringToWString(name).c_str());
#endif
    }

    inline void SetThreadName(const char* name)
    {
#if defined(WIN32) || defined(_WIN32)
        SetThreadName(GetCurrentThread(), name);
#endif
    }
} // namespace ego::threadUtils
