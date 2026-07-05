#include "InputController.h"

#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/Platform/Input/InputDeviceController.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Event/EventController.h"
#include "EgoRuntime/RuntimeContext.h"

#include "InputEvents.h"

namespace
{
    ego::EventController& GetCurrentEventController()
    {
        return ego::context::GetRuntimeContext().getEventController();
    }

    bool IsValueChanged(ego::InputDeviceKeyValue _value, ego::InputDeviceKeyValue _previousValue)
    {
        return _value != _previousValue;
    }

    bool IsButtonDown(ego::InputDeviceKeyValue _value)
    {
        return _value > ego::MinInputDeviceKeyValue;
    }

    bool HasDeviceChanges(const ego::InputDevicePointer& _device)
    {
        EGO_ASSERT(_device);

        const ego::InputDeviceKey valueCount = _device->getValueCount();
        for (ego::InputDeviceKey key = 0; key < valueCount; ++key)
        {
            if (IsValueChanged(_device->getValue(key), _device->getPreviousValue(key)))
            {
                return true;
            }
        }

        return false;
    }

    template <typename TEvent>
    void EmitInputDeviceEvent(ego::EventController& _eventController, const TEvent& _event)
    {
        _eventController.emitEvent<ego::InputDeviceEvent, TEvent>(_event);
        _eventController.emitEvent(_event);
    }

    template <typename TEvent>
    void EmitInputKeyEvent(ego::EventController& _eventController, const TEvent& _event)
    {
        _eventController.emitEvent<ego::InputKeyEvent, TEvent>(_event);
        _eventController.emitEvent(_event);
    }
} // namespace

bool ego::InputController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(registerInputEvents());
    m_isInitialized = true;

    return true;
}

void ego::InputController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    unregisterInputEvents();
    m_isInitialized = false;
}

void ego::InputController::update() const
{
    EGO_CHECK_RETURN(m_isInitialized);

    EventController& eventController = GetCurrentEventController();
    const InputDeviceController::DeviceCollection& devices =
        context::GetPlatform().getInputDeviceController().getDevices();
    for (const InputDevicePointer& device : devices)
    {
        emitDeviceEvents(eventController, device);
    }
}

bool ego::InputController::isInitialized() const
{
    return m_isInitialized;
}

bool ego::InputController::registerInputEvents()
{
    EventController& eventController = GetCurrentEventController();

    EGO_CHECK_RETURN_FALSE(eventController.registerEvent<InputDeviceEvent>());
    if (!eventController.registerEvent<InputDeviceChangedEvent>())
    {
        eventController.unregisterEvent<InputDeviceEvent>();
        return false;
    }

    if (!eventController.registerEvent<InputKeyEvent>())
    {
        eventController.unregisterEvent<InputDeviceChangedEvent>();
        eventController.unregisterEvent<InputDeviceEvent>();
        return false;
    }

    if (!eventController.registerEvent<InputKeyChangedEvent>())
    {
        eventController.unregisterEvent<InputKeyEvent>();
        eventController.unregisterEvent<InputDeviceChangedEvent>();
        eventController.unregisterEvent<InputDeviceEvent>();
        return false;
    }

    if (!eventController.registerEvent<InputButtonPressedEvent>())
    {
        eventController.unregisterEvent<InputKeyChangedEvent>();
        eventController.unregisterEvent<InputKeyEvent>();
        eventController.unregisterEvent<InputDeviceChangedEvent>();
        eventController.unregisterEvent<InputDeviceEvent>();
        return false;
    }

    if (!eventController.registerEvent<InputButtonReleasedEvent>())
    {
        eventController.unregisterEvent<InputButtonPressedEvent>();
        eventController.unregisterEvent<InputKeyChangedEvent>();
        eventController.unregisterEvent<InputKeyEvent>();
        eventController.unregisterEvent<InputDeviceChangedEvent>();
        eventController.unregisterEvent<InputDeviceEvent>();
        return false;
    }

    return true;
}

void ego::InputController::unregisterInputEvents()
{
    EventController& eventController = GetCurrentEventController();

    eventController.unregisterEvent<InputButtonReleasedEvent>();
    eventController.unregisterEvent<InputButtonPressedEvent>();
    eventController.unregisterEvent<InputKeyChangedEvent>();
    eventController.unregisterEvent<InputKeyEvent>();
    eventController.unregisterEvent<InputDeviceChangedEvent>();
    eventController.unregisterEvent<InputDeviceEvent>();
}

void ego::InputController::emitDeviceEvents(EventController& _eventController, const InputDevicePointer& _device) const
{
    EGO_CHECK_RETURN(_device);

    if (HasDeviceChanges(_device))
    {
        const InputDeviceChangedEvent deviceChangedEvent(_device);
        EmitInputDeviceEvent(_eventController, deviceChangedEvent);
    }

    const InputDeviceKey valueCount = _device->getValueCount();
    for (InputDeviceKey key = 0; key < valueCount; ++key)
    {
        emitKeyEvents(
            _eventController,
            _device,
            key,
            _device->getKeyType(key),
            _device->getValue(key),
            _device->getPreviousValue(key));
    }
}

void ego::InputController::emitKeyEvents(
    EventController& _eventController,
    const InputDevicePointer& _device,
    InputDeviceKey _key,
    InputDeviceKeyType _keyType,
    InputDeviceKeyValue _value,
    InputDeviceKeyValue _previousValue) const
{
    if (!IsValueChanged(_value, _previousValue))
    {
        return;
    }

    const InputKeyChangedEvent changedEvent(_device, _key, _keyType, _value, _previousValue);
    EmitInputKeyEvent(_eventController, changedEvent);

    if (_keyType != InputDeviceKeyType::Button)
    {
        return;
    }

    const bool isPressed = IsButtonDown(_value);
    const bool wasPressed = IsButtonDown(_previousValue);
    if (isPressed && !wasPressed)
    {
        const InputButtonPressedEvent pressedEvent(_device, _key, _value, _previousValue);
        EmitInputKeyEvent(_eventController, pressedEvent);
    }
    else if (!isPressed && wasPressed)
    {
        const InputButtonReleasedEvent releasedEvent(_device, _key, _value, _previousValue);
        EmitInputKeyEvent(_eventController, releasedEvent);
    }
}
