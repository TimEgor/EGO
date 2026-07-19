#include "ApplicationWindowGuiViewportEventSource.h"

#include <iterator>
#include <utility>

#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventSubsystem.h"

#include "EgoInput/InputEvents.h"

#include "EgoApplication/Window/ApplicationWindowEvents.h"

namespace
{
    constexpr uint8_t LeftShiftModifierMask = 1u << 0;
    constexpr uint8_t RightShiftModifierMask = 1u << 1;
    constexpr uint8_t LeftControlModifierMask = 1u << 2;
    constexpr uint8_t RightControlModifierMask = 1u << 3;
    constexpr uint8_t LeftAltModifierMask = 1u << 4;
    constexpr uint8_t RightAltModifierMask = 1u << 5;

    constexpr uint8_t ShiftModifierMask = LeftShiftModifierMask | RightShiftModifierMask;
    constexpr uint8_t ControlModifierMask = LeftControlModifierMask | RightControlModifierMask;
    constexpr uint8_t AltModifierMask = LeftAltModifierMask | RightAltModifierMask;

    uint8_t GetKeyboardModifierMask(ego::KeyboardInputKey _key)
    {
        switch (_key)
        {
        case ego::KeyboardInputKey::LeftShift:
            return LeftShiftModifierMask;
        case ego::KeyboardInputKey::RightShift:
            return RightShiftModifierMask;
        case ego::KeyboardInputKey::LeftControl:
            return LeftControlModifierMask;
        case ego::KeyboardInputKey::RightControl:
            return RightControlModifierMask;
        case ego::KeyboardInputKey::LeftAlt:
            return LeftAltModifierMask;
        case ego::KeyboardInputKey::RightAlt:
            return RightAltModifierMask;
        default:
            return 0;
        }
    }

    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }

    ego::gui::Position GetMousePosition(const ego::InputDevice& _device)
    {
        return ego::gui::Position(_device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)), _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)));
    }

    bool HasMousePositionChanged(const ego::InputDevice& _device)
    {
        return _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) != _device.getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) ||
               _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)) != _device.getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisY));
    }

    uint8_t GetMouseButtonMask(ego::MouseInputKey _key)
    {
        const uint32_t buttonIndex = static_cast<uint32_t>(_key) - static_cast<uint32_t>(ego::MouseInputKey::ButtonLeft);
        return static_cast<uint8_t>(1u << buttonIndex);
    }

    void RemoveEventCallback(const ego::EventControllerPointer& _controller, ego::EventCallbackID& _callbackID)
    {
        if (_controller && _callbackID != ego::InvalidEventCallbackID)
        {
            _controller->removeEventCallback(_callbackID);
        }
        _callbackID = ego::InvalidEventCallbackID;
    }

    void RemoveInstancedEventCallback(const ego::EventControllerPointer& _controller, ego::InstancedEventCallbackID& _callbackID)
    {
        if (_controller && _callbackID != ego::InvalidInstancedEventCallbackID)
        {
            _controller->removeInstancedEventDispatcher(_callbackID);
        }
        _callbackID = ego::InvalidInstancedEventCallbackID;
    }
} // namespace

ego::application::ApplicationWindowGuiViewportEventSource::~ApplicationWindowGuiViewportEventSource()
{
    release();
}

bool ego::application::ApplicationWindowGuiViewportEventSource::init(const ApplicationWindowPointer& _window)
{
    EGO_CHECK_INITIALIZATION(!m_window && !m_eventController);
    EGO_CHECK_INITIALIZATION(_window && _window->isValid());

    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    EGO_CHECK_INITIALIZATION(eventSubsystem);

    m_eventController = eventSubsystem->getEventControllerPointer();
    EGO_CHECK_INITIALIZATION(m_eventController);

    m_window = _window;
    EGO_CHECK_RETURN_CALL_FALSE(registerEventCallbacks(), release());
    return true;
}

void ego::application::ApplicationWindowGuiViewportEventSource::release()
{
    unregisterEventCallbacks();

    m_input.clear();
    m_modifiers = gui::InputModifiers();
    m_pressedKeyboardModifiers = 0;
    m_pressedMouseButtons = 0;
    m_isWindowActive = false;
    m_isPointerInsideWindow = false;

    m_eventController = nullptr;
    m_window = nullptr;
}

void ego::application::ApplicationWindowGuiViewportEventSource::drainInput(gui::InputEventCollection& _input)
{
    _input.insert(_input.end(), std::make_move_iterator(m_input.begin()), std::make_move_iterator(m_input.end()));
    m_input.clear();
}

bool ego::application::ApplicationWindowGuiViewportEventSource::registerEventCallbacks()
{
    EGO_CHECK_RETURN_FALSE(m_window && m_eventController);

    m_callbackIDs.m_keyboardInput = m_eventController->addInstanceEventCallback<ApplicationWindowKeyboardInputEvent>(
        m_window->getKeyboardInputEventID(),
        [this](const ApplicationWindowKeyboardInputEvent& _event)
        {
            handleWindowKeyboardInputEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_keyboardInput != InvalidInstancedEventCallbackID);

    m_callbackIDs.m_textInput = m_eventController->addInstanceEventCallback<ApplicationWindowTextInputEvent>(
        m_window->getTextInputEventID(),
        [this](const ApplicationWindowTextInputEvent& _event)
        {
            handleWindowTextInputEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_textInput != InvalidInstancedEventCallbackID);

    m_callbackIDs.m_windowActivation = m_eventController->addEventCallback<ApplicationWindowActivationEvent>(
        [this](const ApplicationWindowActivationEvent& _event)
        {
            handleWindowActivationEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_windowActivation != InvalidEventCallbackID);

    m_callbackIDs.m_mouseChanged = m_eventController->addEventCallback<InputDeviceChangedEvent>(
        [this](const InputDeviceChangedEvent& _event)
        {
            handleMouseChangedEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_mouseChanged != InvalidEventCallbackID);

    m_callbackIDs.m_mouseWheel = m_eventController->addEventCallback<InputKeyChangedEvent>(
        [this](const InputKeyChangedEvent& _event)
        {
            handleMouseWheelEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_mouseWheel != InvalidEventCallbackID);

    m_callbackIDs.m_mouseButtonPressed = m_eventController->addEventCallback<InputButtonPressedEvent>(
        [this](const InputButtonPressedEvent& _event)
        {
            handleMouseButtonEvent(_event, InputButtonAction::Pressed);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_mouseButtonPressed != InvalidEventCallbackID);

    m_callbackIDs.m_mouseButtonReleased = m_eventController->addEventCallback<InputButtonReleasedEvent>(
        [this](const InputButtonReleasedEvent& _event)
        {
            handleMouseButtonEvent(_event, InputButtonAction::Released);
        });
    return m_callbackIDs.m_mouseButtonReleased != InvalidEventCallbackID;
}

void ego::application::ApplicationWindowGuiViewportEventSource::unregisterEventCallbacks()
{
    RemoveEventCallback(m_eventController, m_callbackIDs.m_mouseButtonReleased);
    RemoveEventCallback(m_eventController, m_callbackIDs.m_mouseButtonPressed);
    RemoveEventCallback(m_eventController, m_callbackIDs.m_mouseWheel);
    RemoveEventCallback(m_eventController, m_callbackIDs.m_mouseChanged);
    RemoveEventCallback(m_eventController, m_callbackIDs.m_windowActivation);
    RemoveInstancedEventCallback(m_eventController, m_callbackIDs.m_textInput);
    RemoveInstancedEventCallback(m_eventController, m_callbackIDs.m_keyboardInput);
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleWindowActivationEvent(const ApplicationWindowActivationEvent& _event)
{
    if (!m_window || _event.m_window.get() != m_window.get())
    {
        return;
    }

    m_isWindowActive = _event.m_isActive;
    if (m_isWindowActive)
    {
        return;
    }

    m_pressedMouseButtons = 0;
    m_modifiers = gui::InputModifiers();
    m_pressedKeyboardModifiers = 0;
    m_isPointerInsideWindow = false;

    m_input.push_back(new gui::ViewportDeactivatedEvent());
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleWindowKeyboardInputEvent(const ApplicationWindowKeyboardInputEvent& _event)
{
    updateModifiers(_event.m_inputData);

    if (_event.m_inputData.m_key == KeyboardInputKey::Undefined)
    {
        return;
    }

    gui::KeyEvent event;
    event.m_key = _event.m_inputData.m_key;
    event.m_action = _event.m_inputData.m_action;
    event.m_modifiers = m_modifiers;
    m_input.push_back(new gui::KeyEvent(event));
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleWindowTextInputEvent(const ApplicationWindowTextInputEvent& _event)
{
    if (_event.m_inputData.m_codepoint == 0)
    {
        return;
    }

    gui::TextInputEvent event;
    event.m_codepoint = _event.m_inputData.m_codepoint;
    event.m_modifiers = m_modifiers;
    m_input.push_back(new gui::TextInputEvent(event));
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleMouseChangedEvent(const InputDeviceChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || !HasMousePositionChanged(*_event.m_device))
    {
        return;
    }

    gui::PointerMoveEvent event;
    event.m_position = GetMousePosition(*_event.m_device);
    enqueuePointerInput(std::move(event));
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleMouseWheelEvent(const InputKeyChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || _event.m_key != ToInputDeviceKey(MouseInputKey::Wheel))
    {
        return;
    }

    gui::MouseWheelEvent event;
    event.m_position = GetMousePosition(*_event.m_device);
    event.m_wheelDelta = _event.m_value - _event.m_previousValue;
    enqueuePointerInput(std::move(event));
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleMouseButtonEvent(const InputKeyEvent& _event, InputButtonAction _action)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device)
    {
        return;
    }

    if (_event.m_key < ToInputDeviceKey(MouseInputKey::ButtonLeft) || _event.m_key > ToInputDeviceKey(MouseInputKey::ButtonEight))
    {
        return;
    }

    const MouseInputKey key = static_cast<MouseInputKey>(_event.m_key);
    gui::MouseButtonEvent event;
    event.m_position = GetMousePosition(*_event.m_device);
    event.m_key = key;
    event.m_action = _action;

    const uint8_t mouseButtonMask = GetMouseButtonMask(key);
    const bool inputEnqueued = enqueuePointerInput(std::move(event));
    if (_action == InputButtonAction::Pressed && inputEnqueued)
    {
        m_pressedMouseButtons |= mouseButtonMask;
    }
    else if (_action == InputButtonAction::Released)
    {
        m_pressedMouseButtons &= static_cast<uint8_t>(~mouseButtonMask);
    }
}

bool ego::application::ApplicationWindowGuiViewportEventSource::enqueuePointerInput(gui::PointerMoveEvent _event)
{
    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        if (!emitPointerExit)
        {
            return false;
        }

        gui::PointerExitEvent pointerExitEvent;
        pointerExitEvent.m_position = _event.m_position;
        pointerExitEvent.m_modifiers = m_modifiers;
        m_input.push_back(new gui::PointerExitEvent(pointerExitEvent));
        return true;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(new gui::PointerMoveEvent(std::move(_event)));
    return true;
}

bool ego::application::ApplicationWindowGuiViewportEventSource::enqueuePointerInput(gui::MouseButtonEvent _event)
{
    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        return false;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(new gui::MouseButtonEvent(std::move(_event)));
    return true;
}

bool ego::application::ApplicationWindowGuiViewportEventSource::enqueuePointerInput(gui::MouseWheelEvent _event)
{
    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        return false;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(new gui::MouseWheelEvent(std::move(_event)));
    return true;
}

bool ego::application::ApplicationWindowGuiViewportEventSource::preparePointerInput(gui::Position& _position, bool& _emitPointerExit)
{
    _emitPointerExit = false;
    if (!m_isWindowActive)
    {
        m_isPointerInsideWindow = false;
        return false;
    }

    bool isPointerInsideWindow = false;
    if (!convertPointerPosition(_position, isPointerInsideWindow))
    {
        return false;
    }

    const bool pointerExitedWindow = m_isPointerInsideWindow && !isPointerInsideWindow;
    const bool hasPressedMouseButton = m_pressedMouseButtons != 0;
    m_isPointerInsideWindow = isPointerInsideWindow;
    if (isPointerInsideWindow || hasPressedMouseButton)
    {
        return true;
    }

    _emitPointerExit = pointerExitedWindow;
    return false;
}

void ego::application::ApplicationWindowGuiViewportEventSource::updateModifiers(const WindowKeyboardInputData& _inputData)
{
    const uint8_t modifierMask = GetKeyboardModifierMask(_inputData.m_key);
    if (modifierMask == 0)
    {
        return;
    }

    const bool isPressed = _inputData.m_action == InputButtonAction::Pressed;
    if (isPressed)
    {
        m_pressedKeyboardModifiers |= modifierMask;
    }
    else
    {
        m_pressedKeyboardModifiers &= static_cast<uint8_t>(~modifierMask);
    }

    m_modifiers.m_shift = (m_pressedKeyboardModifiers & ShiftModifierMask) != 0;
    m_modifiers.m_control = (m_pressedKeyboardModifiers & ControlModifierMask) != 0;
    m_modifiers.m_alt = (m_pressedKeyboardModifiers & AltModifierMask) != 0;
}

bool ego::application::ApplicationWindowGuiViewportEventSource::convertPointerPosition(gui::Position& _position, bool& _isInsideWindow) const
{
    EGO_CHECK_RETURN_FALSE(m_window);

    const WindowPoint screenPoint(static_cast<int32_t>(_position.m_x), static_cast<int32_t>(_position.m_y));
    WindowPoint clientPoint;
    EGO_CHECK_RETURN_FALSE(m_window->screenToClient(screenPoint, clientPoint));

    _position = gui::Position(static_cast<float>(clientPoint.m_x), static_cast<float>(clientPoint.m_y));

    const WindowSize& clientSize = m_window->getSize();
    _isInsideWindow =
        clientPoint.m_x >= 0 && clientPoint.m_y >= 0 && clientPoint.m_x < static_cast<int32_t>(clientSize.m_x) && clientPoint.m_y < static_cast<int32_t>(clientSize.m_y);
    return true;
}
