#pragma once

#include "InputDevice.h"

namespace ego
{
    enum class MouseInputKey : InputDeviceKey
    {
        AxisX,
        AxisY,
        Wheel,
        ButtonLeft,
        ButtonRight,
        ButtonMiddle,
        ButtonFour,
        ButtonFive,
        ButtonSix,
        ButtonSeven,
        ButtonEight,
        Count
    };

    inline constexpr InputDeviceKey MouseInputKeyCount = static_cast<InputDeviceKey>(MouseInputKey::Count);

    class MouseInputDevice : public InputDevice
    {
    public:
        MouseInputDevice();
        ~MouseInputDevice() override = default;

        InputDeviceType getType() const override;

        EGO_RTTI_VIRTUAL(MouseInputDevice, InputDevice);
    };

    EGO_POINTER(MouseInputDevice);
} // namespace ego
