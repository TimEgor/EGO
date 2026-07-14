#pragma once

#include <vector>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"
#include "InputDevice.h"

namespace ego
{
    class InputDeviceProvider
    {
    public:
        using DeviceCollection = std::vector<InputDevicePointer>;

        InputDeviceProvider() = default;
        virtual ~InputDeviceProvider() = default;

        virtual DeviceCollection createDevices() = 0;

        EGO_RTTI_VIRTUAL_BASE(InputDeviceProvider);
    };

    EGO_POINTER(InputDeviceProvider);
} // namespace ego
