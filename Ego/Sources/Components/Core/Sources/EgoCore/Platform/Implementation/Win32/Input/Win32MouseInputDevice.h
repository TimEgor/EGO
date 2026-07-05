#pragma once

#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/Platform/PlatformMacros.h"

namespace ego::win32
{
    class Win32MouseInputDevice final : public MouseInputDevice
    {
    public:
        Win32MouseInputDevice();
        ~Win32MouseInputDevice() override = default;

        void update() override;

        EGO_RTTI_VIRTUAL(Win32MouseInputDevice, MouseInputDevice);

    private:
        void updateCursorAxes();
        void updateButtons();
        void resetValues();
    };

    EGO_POINTER(Win32MouseInputDevice);
} // namespace ego::win32
