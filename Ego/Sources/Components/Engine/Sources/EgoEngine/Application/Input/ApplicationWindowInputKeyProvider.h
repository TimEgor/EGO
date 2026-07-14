#pragma once

#include <array>

#include "EgoCore/Platform/Input/KeyboardInputDevice.h"

#include "EgoEvent/EventController.h"

#include "EgoInput/InputKeyProvider.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    struct ApplicationWindowActivationEvent;
    struct ApplicationWindowKeyboardInputEvent;
} // namespace ego::application

namespace ego::engine
{
    class ApplicationWindowInputKeyProvider final : public InputKeyProvider
    {
    public:
        ApplicationWindowInputKeyProvider();
        ~ApplicationWindowInputKeyProvider() override;

        bool init(const application::ApplicationWindowPointer& _window, const EventControllerPointer& _eventController);
        void release();

        void updateDevices();

        const DeviceCollection& getDevices() const override;

        EGO_RTTI_VIRTUAL(ApplicationWindowInputKeyProvider, InputKeyProvider);

    private:
        using KeyValueCollection = std::array<InputDeviceKeyValue, KeyboardInputKeyCount>;

        bool registerEventCallbacks();
        void unregisterEventCallbacks();

        void resetKeyValues();
        void handleApplicationWindowKeyboardInputEvent(const application::ApplicationWindowKeyboardInputEvent& _event);
        void handleApplicationWindowActivationEvent(const application::ApplicationWindowActivationEvent& _event);

        KeyValueCollection m_keyValues;
        application::ApplicationWindowWeakPointer m_window;
        EventControllerPointer m_eventController = nullptr;
        KeyboardInputDevicePointer m_keyboardDevice = nullptr;
        DeviceCollection m_devices;
        InstancedEventCallbackID m_keyboardInputEventCallbackID = InvalidInstancedEventCallbackID;
        EventCallbackID m_windowActivationEventCallbackID = InvalidEventCallbackID;
        bool m_isInitialized = false;
    };

    EGO_POINTER(ApplicationWindowInputKeyProvider);
} // namespace ego::engine
