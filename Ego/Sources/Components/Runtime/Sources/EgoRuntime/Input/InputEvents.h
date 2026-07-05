#pragma once

#include "EgoCore/Platform/Input/InputDevice.h"

#include "EgoRuntime/Event/Event.h"

namespace ego
{
    struct InputDeviceEvent : public Event
    {
        InputDevicePointer m_device;
        InputDeviceType m_deviceType = InvalidInputDeviceType;

        explicit InputDeviceEvent(const InputDevicePointer& _device)
            : m_device(_device),
              m_deviceType(_device ? _device->getType() : InvalidInputDeviceType)
        {
        }

        EGO_EVENT(InputDeviceEvent, Event);
    };

    struct InputDeviceChangedEvent final : public InputDeviceEvent
    {
        explicit InputDeviceChangedEvent(const InputDevicePointer& _device)
            : InputDeviceEvent(_device)
        {
        }

        EGO_EVENT(InputDeviceChangedEvent, InputDeviceEvent);
    };

    struct InputKeyEvent : public Event
    {
        InputDevicePointer m_device;
        InputDeviceType m_deviceType = InvalidInputDeviceType;
        InputDeviceKey m_key = 0;
        InputDeviceKeyType m_keyType = InputDeviceKeyType::Button;
        InputDeviceKeyValue m_value = MinInputDeviceKeyValue;
        InputDeviceKeyValue m_previousValue = MinInputDeviceKeyValue;

        InputKeyEvent(
            const InputDevicePointer& _device,
            InputDeviceKey _key,
            InputDeviceKeyType _keyType,
            InputDeviceKeyValue _value,
            InputDeviceKeyValue _previousValue)
            : m_device(_device),
              m_deviceType(_device ? _device->getType() : InvalidInputDeviceType),
              m_key(_key),
              m_keyType(_keyType),
              m_value(_value),
              m_previousValue(_previousValue)
        {
        }

        EGO_EVENT(InputKeyEvent, Event);
    };

    struct InputKeyChangedEvent final : public InputKeyEvent
    {
        InputKeyChangedEvent(
            const InputDevicePointer& _device,
            InputDeviceKey _key,
            InputDeviceKeyType _keyType,
            InputDeviceKeyValue _value,
            InputDeviceKeyValue _previousValue)
            : InputKeyEvent(_device, _key, _keyType, _value, _previousValue)
        {
        }

        EGO_EVENT(InputKeyChangedEvent, InputKeyEvent);
    };

    struct InputButtonPressedEvent final : public InputKeyEvent
    {
        InputButtonPressedEvent(
            const InputDevicePointer& _device,
            InputDeviceKey _key,
            InputDeviceKeyValue _value,
            InputDeviceKeyValue _previousValue)
            : InputKeyEvent(_device, _key, InputDeviceKeyType::Button, _value, _previousValue)
        {
        }

        EGO_EVENT(InputButtonPressedEvent, InputKeyEvent);
    };

    struct InputButtonReleasedEvent final : public InputKeyEvent
    {
        InputButtonReleasedEvent(
            const InputDevicePointer& _device,
            InputDeviceKey _key,
            InputDeviceKeyValue _value,
            InputDeviceKeyValue _previousValue)
            : InputKeyEvent(_device, _key, InputDeviceKeyType::Button, _value, _previousValue)
        {
        }

        EGO_EVENT(InputButtonReleasedEvent, InputKeyEvent);
    };
} // namespace ego
