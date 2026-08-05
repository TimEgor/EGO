#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Input/InputDeviceController.h"

#include "EgoInput/InputKeyProvider.h"

namespace ego::runtime
{
    class RuntimeInputKeyProvider final : public InputKeyProvider, public NonCopyable
    {
    public:
        explicit RuntimeInputKeyProvider(InputDeviceController& _inputDeviceController);
        ~RuntimeInputKeyProvider() override;

        void updateDevices() override;
        const DeviceCollection& getDevices() const override;

        EGO_RTTI_VIRTUAL(RuntimeInputKeyProvider, InputKeyProvider);

    private:
        InputDeviceController& m_inputDeviceController;
    };

    EGO_POINTER(RuntimeInputKeyProvider);
} // namespace ego::runtime
