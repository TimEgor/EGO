#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/PlatformMacros.h"

namespace ego::win32
{
    class Win32SurfaceUtils final : public NonInstanceable
    {
    public:
        static KeyboardInputKey ToKeyboardKey(WPARAM _key, LPARAM _lParam);
        static bool IsRepeatedKey(LPARAM _lParam);

    private:
        static KeyboardInputKey OffsetKeyboardKey(KeyboardInputKey _firstKey, WPARAM _offset);
        static bool IsExtendedKey(LPARAM _lParam);
        static WPARAM ResolveModifierKey(WPARAM _key, LPARAM _lParam);

        static constexpr int32_t KeyScanCodeOffset = 16;
        static constexpr LPARAM KeyRepeatMask = static_cast<LPARAM>(1) << 30;
        static constexpr LPARAM KeyExtendedMask = static_cast<LPARAM>(1) << 24;
    };
} // namespace ego::win32
