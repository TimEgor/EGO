#pragma once

#include <vector>

#include "EgoCore/Platform/Input/InputDevice.h"
#include "EgoCore/Reference/Pointer.h"

#include "InputKeyProvider.h"

namespace ego
{
    class EventController;
    EGO_POINTER(EventController);

    class InputController final
    {
    public:
        InputController() = default;
        ~InputController();

        bool init();
        void release();

        void update() const;

        bool registerKeyProvider(const InputKeyProviderPointer& _provider);
        bool unregisterKeyProvider(const InputKeyProviderPointer& _provider);

        bool isInitialized() const;

        EventControllerPointer getEventControllerPointer() const;
        EventController& getEventController() const;

    private:
        using KeyProviderCollection = std::vector<InputKeyProviderPointer>;

        bool registerInputEvents();
        void unregisterInputEvents();

        bool hasKeyProvider(const InputKeyProviderPointer& _provider) const;

        void emitDeviceEvents(EventController& _eventController, const InputDevicePointer& _device) const;
        void emitKeyProviderDeviceEvents(EventController& _eventController, const InputKeyProviderPointer& _provider) const;
        void emitKeyEvents(
            EventController& _eventController,
            const InputDevicePointer& _device,
            InputDeviceKey _key,
            InputDeviceKeyType _keyType,
            InputDeviceKeyValue _value,
            InputDeviceKeyValue _previousValue) const;

        KeyProviderCollection m_keyProviders;
        EventControllerPointer m_eventController = nullptr;
        bool m_isInitialized = false;
    };

    EGO_POINTER(InputController);
} // namespace ego
