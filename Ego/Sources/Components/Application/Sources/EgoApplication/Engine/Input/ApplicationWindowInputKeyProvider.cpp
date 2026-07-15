#include "ApplicationWindowInputKeyProvider.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoApplication/Window/ApplicationWindowEvents.h"

namespace
{
    class ApplicationKeyboardInputDevice final : public ego::KeyboardInputDevice
    {
    public:
        ApplicationKeyboardInputDevice() = default;
        ~ApplicationKeyboardInputDevice() override = default;

        void update() override {}

        EGO_RTTI_VIRTUAL(ApplicationKeyboardInputDevice, ego::KeyboardInputDevice);
    };

    ego::InputDeviceKeyValue GetKeyValue(const ego::WindowKeyboardInputData& _inputData)
    {
        if (_inputData.m_action == ego::WindowKeyboardInputAction::Pressed)
        {
            return ego::MaxInputDeviceFixedKeyValue;
        }

        return ego::MinInputDeviceKeyValue;
    }
} // namespace

ego::application::ApplicationWindowInputKeyProvider::ApplicationWindowInputKeyProvider()
{
    resetKeyValues();
}

ego::application::ApplicationWindowInputKeyProvider::~ApplicationWindowInputKeyProvider()
{
    release();
}

bool ego::application::ApplicationWindowInputKeyProvider::init(const ApplicationWindowPointer& _window, const EventControllerPointer& _eventController)
{
    if (m_isInitialized)
    {
        const application::ApplicationWindowPointer window = m_window.lock();
        return window && window.get() == _window.get() && m_eventController.get() == _eventController.get();
    }

    EGO_CHECK_INITIALIZATION(_window && _window->isValid());
    EGO_CHECK_INITIALIZATION(_eventController);

    m_window = _window;
    m_eventController = _eventController;
    resetKeyValues();
    m_keyboardDevice = new ApplicationKeyboardInputDevice();
    EGO_CHECK_INITIALIZATION(m_keyboardDevice);
    m_devices.push_back(m_keyboardDevice);
    EGO_CHECK_INITIALIZATION(registerEventCallbacks());
    m_isInitialized = true;

    return true;
}

void ego::application::ApplicationWindowInputKeyProvider::release()
{
    unregisterEventCallbacks();
    resetKeyValues();
    m_devices.clear();
    m_keyboardDevice = nullptr;
    m_window.reset();
    m_eventController = nullptr;
    m_isInitialized = false;
}

const ego::InputKeyProvider::DeviceCollection& ego::application::ApplicationWindowInputKeyProvider::getDevices() const
{
    return m_devices;
}

void ego::application::ApplicationWindowInputKeyProvider::updateDevices()
{
    if (!m_keyboardDevice)
    {
        return;
    }

    m_keyboardDevice->storePreviousValues();

    for (InputDeviceKey key = 0; key < KeyboardInputKeyCount; ++key)
    {
        m_keyboardDevice->setValue(key, m_keyValues[key]);
    }
}

bool ego::application::ApplicationWindowInputKeyProvider::registerEventCallbacks()
{
    EGO_CHECK_RETURN_FALSE(m_eventController);

    const application::ApplicationWindowPointer window = m_window.lock();
    EGO_CHECK_RETURN_FALSE(window);

    m_keyboardInputEventCallbackID = m_eventController->addInstanceEventCallback<application::ApplicationWindowKeyboardInputEvent>(
        window->getKeyboardInputEventID(),
        [this](const application::ApplicationWindowKeyboardInputEvent& _event)
        {
            handleApplicationWindowKeyboardInputEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_keyboardInputEventCallbackID != InvalidInstancedEventCallbackID);

    m_windowActivationEventCallbackID = m_eventController->addEventCallback<application::ApplicationWindowActivationEvent>(
        [this](const application::ApplicationWindowActivationEvent& _event)
        {
            handleApplicationWindowActivationEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_windowActivationEventCallbackID != InvalidEventCallbackID);

    return true;
}

void ego::application::ApplicationWindowInputKeyProvider::unregisterEventCallbacks()
{
    if (!m_eventController)
    {
        m_windowActivationEventCallbackID = InvalidEventCallbackID;
        m_keyboardInputEventCallbackID = InvalidInstancedEventCallbackID;
        return;
    }

    if (m_windowActivationEventCallbackID != InvalidEventCallbackID)
    {
        m_eventController->removeEventCallback(m_windowActivationEventCallbackID);
        m_windowActivationEventCallbackID = InvalidEventCallbackID;
    }

    if (m_keyboardInputEventCallbackID != InvalidInstancedEventCallbackID)
    {
        m_eventController->removeInstancedEventDispatcher(m_keyboardInputEventCallbackID);
        m_keyboardInputEventCallbackID = InvalidInstancedEventCallbackID;
    }
}

void ego::application::ApplicationWindowInputKeyProvider::resetKeyValues()
{
    m_keyValues.fill(MinInputDeviceKeyValue);
}

void ego::application::ApplicationWindowInputKeyProvider::handleApplicationWindowKeyboardInputEvent(const ApplicationWindowKeyboardInputEvent& _event)
{
    const application::ApplicationWindowPointer window = m_window.lock();
    if (!window || _event.m_window.get() != window.get())
    {
        return;
    }

    if (_event.m_inputData.m_key >= KeyboardInputKeyCount)
    {
        return;
    }

    m_keyValues[_event.m_inputData.m_key] = GetKeyValue(_event.m_inputData);
}

void ego::application::ApplicationWindowInputKeyProvider::handleApplicationWindowActivationEvent(const ApplicationWindowActivationEvent& _event)
{
    const application::ApplicationWindowPointer window = m_window.lock();
    if (window && _event.m_window.get() == window.get() && !_event.m_isActive)
    {
        resetKeyValues();
    }
}
