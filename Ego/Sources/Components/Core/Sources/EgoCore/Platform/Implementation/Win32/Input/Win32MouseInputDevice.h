#pragma once

#include <atomic>

#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/Platform/PlatformMacros.h"

namespace ego::win32
{
    class Win32MouseInputDevice final : public MouseInputDevice
    {
    public:
        Win32MouseInputDevice();
        ~Win32MouseInputDevice() override = default;

        static void AddWheelDelta(InputDeviceKeyValue _delta);

        void update() override;

        EGO_RTTI_VIRTUAL(Win32MouseInputDevice, MouseInputDevice);

    private:
        void updateCursorAxes();
        void updateWheel();
        void updateButtons();
        void resetValues();

        static std::atomic<InputDeviceKeyValue> PendingWheelDelta;
    };

    EGO_POINTER(Win32MouseInputDevice);
} // namespace ego::win32
