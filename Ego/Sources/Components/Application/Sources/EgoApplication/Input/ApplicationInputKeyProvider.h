#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Input/InputDeviceController.h"

#include "EgoInput/InputKeyProvider.h"

namespace ego::application
{
    class ApplicationInputKeyProvider final : public InputKeyProvider, public NonCopyable
    {
    public:
        explicit ApplicationInputKeyProvider(InputDeviceController& _inputDeviceController);
        ~ApplicationInputKeyProvider() override;

        void updateDevices() override;
        const DeviceCollection& getDevices() const override;

        EGO_RTTI_VIRTUAL(ApplicationInputKeyProvider, InputKeyProvider);

    private:
        InputDeviceController& m_inputDeviceController;
    };

    EGO_POINTER(ApplicationInputKeyProvider);
} // namespace ego::application
