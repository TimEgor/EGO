#include "ApplicationGuiViewport.h"

#include <iterator>
#include <memory>
#include <utility>

#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventSubsystem.h"

#include "EgoInput/InputEvents.h"

#include "EgoApplication/Presentation/PresentationSurfaceEvents.h"

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

ego::application::ApplicationGuiViewport::~ApplicationGuiViewport()
{
    release();
}

bool ego::application::ApplicationGuiViewport::init(const Presentation& _presentation)
{
    EGO_CHECK_INITIALIZATION(!m_surface);
    EGO_CHECK_INITIALIZATION(_presentation.m_surface && _presentation.m_graphicPresenter);

    m_surface = _presentation.m_surface;
    m_status = gui::ViewportUpdateStatus::Alive;
    updateSize();

    EGO_CHECK_INITIALIZATION(registerEventCallbacks(_presentation.m_surfaceEventIDs));

    return true;
}

void ego::application::ApplicationGuiViewport::release()
{
    unregisterEventCallbacks();

    m_surface = nullptr;
    m_status = gui::ViewportUpdateStatus::CloseRequested;
    m_size = gui::SizeZero;
    m_input.clear();
    resetInput();
}

ego::gui::ViewportUpdate ego::application::ApplicationGuiViewport::poll(const GraphicPresenterPointer& _graphicPresenter)
{
    updateSize();

    gui::ViewportUpdate update;
    update.m_size = m_size;
    update.m_status = m_status;
    update.m_graphicPresenter = _graphicPresenter;
    if (update.m_status == gui::ViewportUpdateStatus::Alive && !_graphicPresenter)
    {
        update.m_status = gui::ViewportUpdateStatus::Lost;
        m_status = update.m_status;
    }

    update.m_input.insert(update.m_input.end(), std::make_move_iterator(m_input.begin()), std::make_move_iterator(m_input.end()));
    m_input.clear();

    return update;
}

const ego::PresentationSurfacePointer& ego::application::ApplicationGuiViewport::getSurfacePointer() const
{
    return m_surface;
}

bool ego::application::ApplicationGuiViewport::registerEventCallbacks(const PresentationSurfaceEventIDs& _eventIDs)
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    unregisterEventCallbacks();

    EGO_CHECK_RETURN_FALSE(_eventIDs.m_destroying != InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(_eventIDs.m_activation != InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(_eventIDs.m_keyboardInput != InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(_eventIDs.m_textInput != InvalidInstancedEventID);

    m_callbackIDs.m_surfaceDestroying = eventController->addInstanceEventCallback<PresentationSurfaceDestroyingEvent>(
        _eventIDs.m_destroying,
        *this,
        &ApplicationGuiViewport::handleSurfaceDestroyingEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_surfaceDestroying != InvalidInstancedEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_surfaceActivation = eventController->addInstanceEventCallback<PresentationSurfaceActivationEvent>(
        _eventIDs.m_activation,
        *this,
        &ApplicationGuiViewport::handleSurfaceActivationEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_surfaceActivation != InvalidInstancedEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_surfaceKeyboardInput = eventController->addInstanceEventCallback<PresentationSurfaceKeyboardInputEvent>(
        _eventIDs.m_keyboardInput,
        *this,
        &ApplicationGuiViewport::handleSurfaceKeyboardInputEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_surfaceKeyboardInput != InvalidInstancedEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_surfaceTextInput = eventController->addInstanceEventCallback<PresentationSurfaceTextInputEvent>(
        _eventIDs.m_textInput,
        *this,
        &ApplicationGuiViewport::handleSurfaceTextInputEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_surfaceTextInput != InvalidInstancedEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_mouseChanged = eventController->addEventCallback<InputDeviceChangedEvent>(*this, &ApplicationGuiViewport::handleMouseChangedEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseChanged != InvalidEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_mouseWheel = eventController->addEventCallback<InputKeyChangedEvent>(*this, &ApplicationGuiViewport::handleMouseWheelEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseWheel != InvalidEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_mouseButtonPressed =
        eventController->addEventCallback<InputButtonPressedEvent>(*this, &ApplicationGuiViewport::handleMouseButtonPressedEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseButtonPressed != InvalidEventCallbackID, unregisterEventCallbacks());

    m_callbackIDs.m_mouseButtonReleased =
        eventController->addEventCallback<InputButtonReleasedEvent>(*this, &ApplicationGuiViewport::handleMouseButtonReleasedEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseButtonReleased != InvalidEventCallbackID, unregisterEventCallbacks());

    return true;
}

void ego::application::ApplicationGuiViewport::unregisterEventCallbacks()
{
    const EventControllerPointer eventController = GetEventControllerPointer();

    RemoveEventCallback(eventController, m_callbackIDs.m_mouseButtonReleased);
    RemoveEventCallback(eventController, m_callbackIDs.m_mouseButtonPressed);
    RemoveEventCallback(eventController, m_callbackIDs.m_mouseWheel);
    RemoveEventCallback(eventController, m_callbackIDs.m_mouseChanged);
    RemoveInstancedEventCallback(eventController, m_callbackIDs.m_surfaceTextInput);
    RemoveInstancedEventCallback(eventController, m_callbackIDs.m_surfaceKeyboardInput);
    RemoveInstancedEventCallback(eventController, m_callbackIDs.m_surfaceActivation);
    RemoveInstancedEventCallback(eventController, m_callbackIDs.m_surfaceDestroying);
}

void ego::application::ApplicationGuiViewport::handleSurfaceDestroyingEvent(const PresentationSurfaceDestroyingEvent& _event)
{
    EGO_CHECK_RETURN(m_surface && _event.m_surface.get() == m_surface.get());

    m_status = gui::ViewportUpdateStatus::CloseRequested;
    resetInput();
}

void ego::application::ApplicationGuiViewport::handleSurfaceActivationEvent(const PresentationSurfaceActivationEvent& _event)
{
    EGO_CHECK_RETURN(m_surface && _event.m_surface.get() == m_surface.get());

    m_isSurfaceActive = _event.m_isActive;
    if (m_isSurfaceActive)
    {
        return;
    }

    resetInput();
    m_input.push_back(std::make_unique<gui::ViewportDeactivatedEvent>());
}

void ego::application::ApplicationGuiViewport::handleSurfaceKeyboardInputEvent(const PresentationSurfaceKeyboardInputEvent& _event)
{
    EGO_CHECK_RETURN(m_surface && _event.m_surface.get() == m_surface.get());

    updateModifiers(_event);
    if (_event.m_key == KeyboardInputKey::Undefined)
    {
        return;
    }

    gui::KeyEvent event;
    event.m_key = _event.m_key;
    event.m_action = _event.m_action;
    event.m_modifiers = m_modifiers;
    m_input.push_back(std::make_unique<gui::KeyEvent>(event));
}

void ego::application::ApplicationGuiViewport::handleSurfaceTextInputEvent(const PresentationSurfaceTextInputEvent& _event)
{
    EGO_CHECK_RETURN(m_surface && _event.m_surface.get() == m_surface.get());
    EGO_CHECK_RETURN(_event.m_codepoint != 0);

    gui::TextInputEvent event;
    event.m_codepoint = _event.m_codepoint;
    event.m_modifiers = m_modifiers;
    m_input.push_back(std::make_unique<gui::TextInputEvent>(event));
}

void ego::application::ApplicationGuiViewport::handleMouseChangedEvent(const InputDeviceChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || !HasMousePositionChanged(*_event.m_device))
    {
        return;
    }

    gui::PointerMoveEvent event;
    event.m_position = GetMousePosition(*_event.m_device);
    enqueuePointerInput(std::move(event));
}

void ego::application::ApplicationGuiViewport::handleMouseWheelEvent(const InputKeyChangedEvent& _event)
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

void ego::application::ApplicationGuiViewport::handleMouseButtonPressedEvent(const InputKeyEvent& _event)
{
    handleMouseButtonEvent(_event, InputButtonAction::Pressed);
}

void ego::application::ApplicationGuiViewport::handleMouseButtonReleasedEvent(const InputKeyEvent& _event)
{
    handleMouseButtonEvent(_event, InputButtonAction::Released);
}

void ego::application::ApplicationGuiViewport::handleMouseButtonEvent(const InputKeyEvent& _event, InputButtonAction _action)
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
    const uint8_t mouseButtonMask = GetMouseButtonMask(key);

    gui::MouseButtonEvent event;
    event.m_position = GetMousePosition(*_event.m_device);
    event.m_key = key;
    event.m_action = _action;

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

void ego::application::ApplicationGuiViewport::updateSize()
{
    if (m_status != gui::ViewportUpdateStatus::Alive || !m_surface)
    {
        return;
    }

    const PresentationSurfaceSize& surfaceSize = m_surface->getSize();
    m_size = gui::Size(static_cast<float>(surfaceSize.m_x), static_cast<float>(surfaceSize.m_y));
}

void ego::application::ApplicationGuiViewport::resetInput()
{
    m_modifiers = gui::InputModifiers();
    m_pressedKeyboardModifiers = 0;
    m_pressedMouseButtons = 0;
    m_isSurfaceActive = false;
    m_isPointerInsideSurface = false;
}

void ego::application::ApplicationGuiViewport::updateModifiers(const PresentationSurfaceKeyboardInputEvent& _event)
{
    const uint8_t modifierMask = GetKeyboardModifierMask(_event.m_key);
    if (modifierMask == 0)
    {
        return;
    }

    const bool isPressed = _event.m_action == InputButtonAction::Pressed;
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

bool ego::application::ApplicationGuiViewport::enqueuePointerInput(gui::PointerMoveEvent _event)
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
        m_input.push_back(std::make_unique<gui::PointerExitEvent>(pointerExitEvent));
        return true;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(std::make_unique<gui::PointerMoveEvent>(std::move(_event)));
    return true;
}

bool ego::application::ApplicationGuiViewport::enqueuePointerInput(gui::MouseButtonEvent _event)
{
    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        return false;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(std::make_unique<gui::MouseButtonEvent>(std::move(_event)));
    return true;
}

bool ego::application::ApplicationGuiViewport::enqueuePointerInput(gui::MouseWheelEvent _event)
{
    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        return false;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(std::make_unique<gui::MouseWheelEvent>(std::move(_event)));
    return true;
}

bool ego::application::ApplicationGuiViewport::preparePointerInput(gui::Position& _position, bool& _emitPointerExit)
{
    _emitPointerExit = false;
    if (m_status != gui::ViewportUpdateStatus::Alive || !m_isSurfaceActive)
    {
        m_isPointerInsideSurface = false;
        return false;
    }

    bool isPointerInsideSurface = false;
    if (!convertPointerPosition(_position, isPointerInsideSurface))
    {
        return false;
    }

    const bool pointerExitedSurface = m_isPointerInsideSurface && !isPointerInsideSurface;
    const bool hasPressedMouseButton = m_pressedMouseButtons != 0;
    m_isPointerInsideSurface = isPointerInsideSurface;
    if (isPointerInsideSurface || hasPressedMouseButton)
    {
        return true;
    }

    _emitPointerExit = pointerExitedSurface;
    return false;
}

bool ego::application::ApplicationGuiViewport::convertPointerPosition(gui::Position& _position, bool& _isInsideSurface) const
{
    EGO_CHECK_RETURN_FALSE(m_surface);

    const PresentationSurfacePoint screenPoint(static_cast<int32_t>(_position.m_x), static_cast<int32_t>(_position.m_y));
    PresentationSurfacePoint localPoint;
    EGO_CHECK_RETURN_FALSE(m_surface->screenToLocal(screenPoint, localPoint));

    _position = gui::Position(static_cast<float>(localPoint.m_x), static_cast<float>(localPoint.m_y));

    const PresentationSurfaceSize& surfaceSize = m_surface->getSize();
    _isInsideSurface = localPoint.m_x >= 0 && localPoint.m_y >= 0 && localPoint.m_x < static_cast<int32_t>(surfaceSize.m_x) &&
                       localPoint.m_y < static_cast<int32_t>(surfaceSize.m_y);
    return true;
}

ego::EventControllerPointer ego::application::ApplicationGuiViewport::GetEventControllerPointer()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    return eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
}
