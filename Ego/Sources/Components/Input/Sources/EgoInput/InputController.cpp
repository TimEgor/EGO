#include "InputController.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Input/InputDeviceController.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventController.h"

#include "InputEvents.h"

namespace
{
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

ego::InputController::~InputController()
{
    release();
}

bool ego::InputController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(!m_eventController);

    m_eventController = new EventController();
    EGO_CHECK_INITIALIZATION(m_eventController && m_eventController->init());
    EGO_CHECK_INITIALIZATION(registerInputEvents());

    m_isInitialized = true;

    return true;
}

void ego::InputController::release()
{
    if (m_eventController)
    {
        unregisterInputEvents();
    }

    m_keyProviders.clear();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_eventController);
    m_isInitialized = false;
}

void ego::InputController::update() const
{
    EGO_CHECK_RETURN(m_isInitialized);
    EGO_CHECK_RETURN(m_eventController);

    EventController& eventController = *m_eventController;
    const InputDeviceController::DeviceCollection& devices = GetPlatform().getInputDeviceController().getDevices();
    for (const InputDevicePointer& device : devices)
    {
        emitDeviceEvents(eventController, device);
    }

    for (const InputKeyProviderPointer& provider : m_keyProviders)
    {
        emitKeyProviderDeviceEvents(eventController, provider);
    }
}

bool ego::InputController::registerKeyProvider(const InputKeyProviderPointer& _provider)
{
    EGO_CHECK_RETURN_FALSE(_provider);

    if (hasKeyProvider(_provider))
    {
        return true;
    }

    m_keyProviders.push_back(_provider);
    return true;
}

bool ego::InputController::unregisterKeyProvider(const InputKeyProviderPointer& _provider)
{
    EGO_CHECK_RETURN_FALSE(_provider);

    for (KeyProviderCollection::iterator providerIt = m_keyProviders.begin(); providerIt != m_keyProviders.end(); ++providerIt)
    {
        if (providerIt->get() == _provider.get())
        {
            m_keyProviders.erase(providerIt);
            return true;
        }
    }

    return false;
}

bool ego::InputController::isInitialized() const
{
    return m_isInitialized;
}

ego::EventControllerPointer ego::InputController::getEventControllerPointer() const
{
    return m_eventController;
}

ego::EventController& ego::InputController::getEventController() const
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

bool ego::InputController::registerInputEvents()
{
    EGO_CHECK_RETURN_FALSE(m_eventController);

    EventController& eventController = *m_eventController;

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
    if (!m_eventController)
    {
        return;
    }

    EventController& eventController = *m_eventController;

    eventController.unregisterEvent<InputButtonReleasedEvent>();
    eventController.unregisterEvent<InputButtonPressedEvent>();
    eventController.unregisterEvent<InputKeyChangedEvent>();
    eventController.unregisterEvent<InputKeyEvent>();
    eventController.unregisterEvent<InputDeviceChangedEvent>();
    eventController.unregisterEvent<InputDeviceEvent>();
}

bool ego::InputController::hasKeyProvider(const InputKeyProviderPointer& _provider) const
{
    for (const InputKeyProviderPointer& provider : m_keyProviders)
    {
        if (provider.get() == _provider.get())
        {
            return true;
        }
    }

    return false;
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
        emitKeyEvents(_eventController, _device, key, _device->getKeyType(key), _device->getValue(key), _device->getPreviousValue(key));
    }
}

void ego::InputController::emitKeyProviderDeviceEvents(EventController& _eventController, const InputKeyProviderPointer& _provider) const
{
    EGO_CHECK_RETURN(_provider);

    const InputKeyProvider::DeviceCollection& devices = _provider->getDevices();
    for (const InputDevicePointer& device : devices)
    {
        emitDeviceEvents(_eventController, device);
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
