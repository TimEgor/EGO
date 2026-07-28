#pragma once

#include <vector>

#include "EgoCore/Platform/Input/InputDevice.h"
#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    class InputKeyProvider
    {
    public:
        using DeviceCollection = std::vector<InputDevicePointer>;

        InputKeyProvider() = default;
        virtual ~InputKeyProvider() = default;

        virtual void updateDevices() {}
        virtual const DeviceCollection& getDevices() const = 0;

        EGO_RTTI_VIRTUAL_BASE(InputKeyProvider);
    };

    EGO_POINTER(InputKeyProvider);
} // namespace ego
