#pragma once

#include "EgoCore/Platform/Input/KeyboardInputDevice.h"

namespace ego::win32
{
    class Win32KeyboardInputDevice final : public KeyboardInputDevice
    {
    public:
        Win32KeyboardInputDevice() = default;
        ~Win32KeyboardInputDevice() override = default;

        void update() override;

        EGO_RTTI_VIRTUAL(Win32KeyboardInputDevice, KeyboardInputDevice);
    };

    EGO_POINTER(Win32KeyboardInputDevice);
} // namespace ego::win32
