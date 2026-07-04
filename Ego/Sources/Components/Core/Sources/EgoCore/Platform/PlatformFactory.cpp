#include "PlatformFactory.h"

#if defined(WIN32) || defined(_WIN32)
    #include "EgoCore/Platform/Implementation/Win32/Win32Platform.h"
#endif

ego::PlatformPointer ego::CreatePlatform(void* _nativeInstanceHandle)
{
#if defined(WIN32) || defined(_WIN32)
    return PlatformPointer(new win32::Win32Platform(static_cast<HINSTANCE>(_nativeInstanceHandle)));
#else
    (void)_nativeInstanceHandle;
    return nullptr;
#endif
}
