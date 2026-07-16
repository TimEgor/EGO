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
    ego::gui::GuiKey ToGuiKey(ego::WindowKeyboardKey _key)
    {
        switch (_key)
        {
        case 0x08:
            return ego::gui::GuiKey::Backspace;
        case 0x09:
            return ego::gui::GuiKey::Tab;
        case 0x0D:
            return ego::gui::GuiKey::Enter;
        case 0x10:
        case 0xA0:
        case 0xA1:
            return ego::gui::GuiKey::Shift;
        case 0x11:
        case 0xA2:
        case 0xA3:
            return ego::gui::GuiKey::Control;
        case 0x1B:
            return ego::gui::GuiKey::Escape;
        case 0x23:
            return ego::gui::GuiKey::End;
        case 0x24:
            return ego::gui::GuiKey::Home;
        case 0x25:
            return ego::gui::GuiKey::Left;
        case 0x27:
            return ego::gui::GuiKey::Right;
        case 0x2E:
            return ego::gui::GuiKey::Delete;
        case 0x41:
            return ego::gui::GuiKey::A;
        default:
            return ego::gui::GuiKey::Undefined;
        }
    }

    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }

    ego::gui::GuiPosition GetMousePosition(const ego::InputDevice& _device)
    {
        return ego::gui::GuiPosition(
            _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)),
            _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)));
    }

    bool HasMousePositionChanged(const ego::InputDevice& _device)
    {
        return _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) != _device.getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) ||
               _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)) != _device.getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisY));
    }

    bool TryGetGuiMouseButton(ego::InputDeviceKey _key, ego::gui::GuiMouseButton& _mouseButton)
    {
        if (_key >= ego::MouseInputKeyCount)
        {
            return false;
        }

        switch (static_cast<ego::MouseInputKey>(_key))
        {
        case ego::MouseInputKey::ButtonLeft:
            _mouseButton = ego::gui::GuiMouseButton::Left;
            return true;
        case ego::MouseInputKey::ButtonRight:
            _mouseButton = ego::gui::GuiMouseButton::Right;
            return true;
        case ego::MouseInputKey::ButtonMiddle:
            _mouseButton = ego::gui::GuiMouseButton::Middle;
            return true;
        default:
            return false;
        }
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

    m_events.clear();
    m_pressedMouseButtonCount = 0;
    m_isWindowActive = false;
    m_isPointerInsideWindow = false;

    m_eventController = nullptr;
    m_window = nullptr;
}

void ego::application::ApplicationWindowGuiViewportEventSource::drainEvents(gui::GuiViewportEventCollection& _events)
{
    _events.insert(_events.end(), std::make_move_iterator(m_events.begin()), std::make_move_iterator(m_events.end()));
    m_events.clear();
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
            handleMouseButtonEvent(_event, gui::GuiInputEventType::MouseButtonDown);
        });
    EGO_CHECK_RETURN_FALSE(m_callbackIDs.m_mouseButtonPressed != InvalidEventCallbackID);

    m_callbackIDs.m_mouseButtonReleased = m_eventController->addEventCallback<InputButtonReleasedEvent>(
        [this](const InputButtonReleasedEvent& _event)
        {
            handleMouseButtonEvent(_event, gui::GuiInputEventType::MouseButtonUp);
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

    m_pressedMouseButtonCount = 0;
    m_isPointerInsideWindow = false;

    gui::GuiInputEvent event;
    event.m_type = gui::GuiInputEventType::FocusLost;
    m_events.push_back(event);
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleWindowKeyboardInputEvent(const ApplicationWindowKeyboardInputEvent& _event)
{
    const gui::GuiKey key = ToGuiKey(_event.m_inputData.m_key);
    if (key == gui::GuiKey::Undefined)
    {
        return;
    }

    gui::GuiInputEvent event;
    event.m_type = _event.m_inputData.m_action == WindowKeyboardInputAction::Pressed ? gui::GuiInputEventType::KeyDown : gui::GuiInputEventType::KeyUp;
    event.m_key = key;
    m_events.push_back(event);
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleWindowTextInputEvent(const ApplicationWindowTextInputEvent& _event)
{
    if (_event.m_inputData.m_codepoint == 0)
    {
        return;
    }

    gui::GuiInputEvent event;
    event.m_type = gui::GuiInputEventType::TextInput;
    event.m_textCodepoint = _event.m_inputData.m_codepoint;
    m_events.push_back(event);
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleMouseChangedEvent(const InputDeviceChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || !HasMousePositionChanged(*_event.m_device))
    {
        return;
    }

    gui::GuiInputEvent event;
    event.m_type = gui::GuiInputEventType::MouseMove;
    event.m_position = GetMousePosition(*_event.m_device);
    enqueuePointerEvent(std::move(event));
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleMouseWheelEvent(const InputKeyChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || _event.m_key != ToInputDeviceKey(MouseInputKey::Wheel))
    {
        return;
    }

    gui::GuiInputEvent event;
    event.m_type = gui::GuiInputEventType::MouseWheel;
    event.m_position = GetMousePosition(*_event.m_device);
    event.m_wheelDelta = _event.m_value - _event.m_previousValue;
    enqueuePointerEvent(std::move(event));
}

void ego::application::ApplicationWindowGuiViewportEventSource::handleMouseButtonEvent(const InputKeyEvent& _event, gui::GuiInputEventType _type)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device)
    {
        return;
    }

    gui::GuiMouseButton mouseButton;
    if (!TryGetGuiMouseButton(_event.m_key, mouseButton))
    {
        return;
    }

    gui::GuiInputEvent event;
    event.m_type = _type;
    event.m_position = GetMousePosition(*_event.m_device);
    event.m_mouseButton = mouseButton;

    const bool eventEnqueued = enqueuePointerEvent(std::move(event));
    if (_type == gui::GuiInputEventType::MouseButtonDown && eventEnqueued)
    {
        ++m_pressedMouseButtonCount;
    }
    else if (_type == gui::GuiInputEventType::MouseButtonUp && eventEnqueued && m_pressedMouseButtonCount > 0)
    {
        --m_pressedMouseButtonCount;
    }
}

bool ego::application::ApplicationWindowGuiViewportEventSource::enqueuePointerEvent(gui::GuiInputEvent _event)
{
    if (!m_isWindowActive)
    {
        m_isPointerInsideWindow = false;
        return false;
    }

    bool isPointerInsideWindow = false;
    if (!convertPointerPosition(_event, isPointerInsideWindow))
    {
        return false;
    }

    const bool pointerLeftWindow = m_isPointerInsideWindow && !isPointerInsideWindow;
    const bool hasPointerCapture = m_pressedMouseButtonCount > 0;
    m_isPointerInsideWindow = isPointerInsideWindow;

    if (!isPointerInsideWindow && !hasPointerCapture)
    {
        if (!pointerLeftWindow || _event.m_type != gui::GuiInputEventType::MouseMove)
        {
            return false;
        }

        _event.m_type = gui::GuiInputEventType::PointerLeave;
    }

    m_events.push_back(std::move(_event));
    return true;
}

bool ego::application::ApplicationWindowGuiViewportEventSource::convertPointerPosition(gui::GuiInputEvent& _event, bool& _isInsideWindow) const
{
    EGO_CHECK_RETURN_FALSE(m_window);

    const WindowPoint screenPoint(static_cast<int32_t>(_event.m_position.m_x), static_cast<int32_t>(_event.m_position.m_y));
    WindowPoint clientPoint;
    EGO_CHECK_RETURN_FALSE(m_window->screenToClient(screenPoint, clientPoint));

    _event.m_position = gui::GuiPosition(static_cast<float>(clientPoint.m_x), static_cast<float>(clientPoint.m_y));

    const WindowSize& clientSize = m_window->getSize();
    _isInsideWindow = clientPoint.m_x >= 0 && clientPoint.m_y >= 0 && clientPoint.m_x < static_cast<int32_t>(clientSize.m_x) &&
                      clientPoint.m_y < static_cast<int32_t>(clientSize.m_y);
    return true;
}
