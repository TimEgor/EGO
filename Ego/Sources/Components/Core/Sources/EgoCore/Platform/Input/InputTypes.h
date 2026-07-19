#pragma once

#include <cstdint>

#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    using InputDeviceType = rtti::TypeMetaInfoID;
    inline constexpr InputDeviceType InvalidInputDeviceType = rtti::InvalidTypeMetaInfoID;

    using InputDeviceKey = uint32_t;
    using InputDeviceKeyValue = float;

    enum class InputDeviceKeyType
    {
        Button,
        Axis,
        Position
    };

    enum class InputButtonAction
    {
        Pressed,
        Released
    };

    inline constexpr InputDeviceKeyValue MinInputDeviceKeyValue = 0.0f;
    inline constexpr InputDeviceKeyValue MaxInputDeviceFixedKeyValue = 1.0f;
} // namespace ego
