#pragma once

#include "EgoCore/Platform/Input/InputDevice.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego
{
    class EventController;

    class InputController final
    {
    public:
        InputController() = default;
        ~InputController()
        {
            release();
        }

        bool init();
        void release();

        void update() const;

        bool isInitialized() const;

    private:
        bool registerInputEvents();
        void unregisterInputEvents();

        void emitDeviceEvents(EventController& _eventController, const InputDevicePointer& _device) const;
        void emitKeyEvents(
            EventController& _eventController,
            const InputDevicePointer& _device,
            InputDeviceKey _key,
            InputDeviceKeyType _keyType,
            InputDeviceKeyValue _value,
            InputDeviceKeyValue _previousValue) const;

        bool m_isInitialized = false;
    };

    EGO_POINTER(InputController);
} // namespace ego
