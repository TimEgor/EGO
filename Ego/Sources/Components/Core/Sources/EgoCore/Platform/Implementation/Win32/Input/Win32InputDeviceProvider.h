#pragma once

#include "EgoCore/Platform/Input/InputDeviceProvider.h"

namespace ego::win32
{
    class Win32InputDeviceProvider final : public InputDeviceProvider
    {
    public:
        Win32InputDeviceProvider() = default;
        ~Win32InputDeviceProvider() override = default;

        DeviceCollection createDevices() override;

        EGO_RTTI_VIRTUAL(Win32InputDeviceProvider, InputDeviceProvider);
    };

    EGO_POINTER(Win32InputDeviceProvider);
} // namespace ego::win32
