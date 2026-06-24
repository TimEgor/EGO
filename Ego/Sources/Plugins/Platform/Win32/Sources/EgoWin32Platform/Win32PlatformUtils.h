#pragma once

#include "EgoEngine/Engine.h"

#include "Win32Platform.h"

namespace ego::win32
{
    inline Win32Platform& GetWin32FrameworkPlatform()
    {
        Platform& platform = engine::GetEngine().getPlatform();
        EGO_ASSERT((rtti::IsObjectBasedOn<Win32Platform>(platform)));

        return reinterpret_cast<Win32Platform&>(platform);
    }
} // namespace ego::win32
