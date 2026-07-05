#pragma once

#include "InputDevice.h"

namespace ego
{
    inline constexpr InputDeviceKey KeyboardInputKeyCount = 256;

    class KeyboardInputDevice : public InputDevice
    {
    public:
        KeyboardInputDevice();
        ~KeyboardInputDevice() override = default;

        InputDeviceType getType() const override;

        EGO_RTTI_VIRTUAL(KeyboardInputDevice, InputDevice);
    };

    EGO_POINTER(KeyboardInputDevice);
} // namespace ego
