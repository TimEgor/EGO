#include "ApplicationGuiViewport.h"

#include <cmath>
#include <iterator>
#include <limits>
#include <utility>

#include "EgoCore/Event/EventSubsystem.h"
#include "EgoCore/UtilsMacros.h"

namespace
{
    constexpr uint8_t LeftShiftModifierMask = 1u << 0;
    constexpr uint8_t RightShiftModifierMask = 1u << 1;
    constexpr uint8_t LeftControlModifierMask = 1u << 2;
    constexpr uint8_t RightControlModifierMask = 1u << 3;
    constexpr uint8_t LeftAltModifierMask = 1u << 4;
    constexpr uint8_t RightAltModifierMask = 1u << 5;
    constexpr uint8_t LeftSuperModifierMask = 1u << 6;
    constexpr uint8_t RightSuperModifierMask = 1u << 7;

    constexpr uint8_t ShiftModifierMask = LeftShiftModifierMask | RightShiftModifierMask;
    constexpr uint8_t ControlModifierMask = LeftControlModifierMask | RightControlModifierMask;
    constexpr uint8_t AltModifierMask = LeftAltModifierMask | RightAltModifierMask;
    constexpr uint8_t SuperModifierMask = LeftSuperModifierMask | RightSuperModifierMask;

    uint16_t ToSurfaceSizeValue(float _value)
    {
        if (!std::isfinite(_value) || _value <= 0.0f)
        {
            return 0;
        }

        const double value = static_cast<double>(_value);
        const double maxValue = static_cast<double>((std::numeric_limits<uint16_t>::max)());

        return value < maxValue ? static_cast<uint16_t>(value) : (std::numeric_limits<uint16_t>::max)();
    }

    int32_t ToSurfacePointValue(float _value)
    {
        if (!std::isfinite(_value))
        {
            return 0;
        }

        const double value = static_cast<double>(_value);
        const double minValue = static_cast<double>((std::numeric_limits<int32_t>::min)());
        const double maxValue = static_cast<double>((std::numeric_limits<int32_t>::max)());
        if (value <= minValue)
        {
            return (std::numeric_limits<int32_t>::min)();
        }

        if (value >= maxValue)
        {
            return (std::numeric_limits<int32_t>::max)();
        }

        return static_cast<int32_t>(value);
    }

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
        case ego::KeyboardInputKey::LeftSystem:
            return LeftSuperModifierMask;
        case ego::KeyboardInputKey::RightSystem:
            return RightSuperModifierMask;
        default:
            return 0;
        }
    }

    uint8_t GetMouseButtonMask(ego::MouseInputKey _key)
    {
        const uint32_t buttonIndex = static_cast<uint32_t>(_key) - static_cast<uint32_t>(ego::MouseInputKey::ButtonLeft);
        return static_cast<uint8_t>(1u << buttonIndex);
    }

} // namespace

ego::application::ApplicationGuiViewport::~ApplicationGuiViewport()
{
    release();
}

bool ego::application::ApplicationGuiViewport::init(const Presentation& _presentation)
{
    EGO_CHECK_INITIALIZATION(!m_presentation.m_surface);
    EGO_CHECK_INITIALIZATION(_presentation.m_surface && _presentation.m_graphicPresenter);

    m_presentation = _presentation;
    m_status = gui::ViewportUpdateStatus::Alive;
    updateBounds();

    EGO_CHECK_RETURN_CALL_FALSE(registerSurfaceEvents(), release());

    return true;
}

void ego::application::ApplicationGuiViewport::release()
{
    if (m_presentation.m_surface)
    {
        unregisterSurfaceEvents();
        m_presentation.m_surface->setInputTransparent(false);
    }

    m_presentation = Presentation();
    m_status = gui::ViewportUpdateStatus::CloseRequested;
    m_position = FloatVector2Zero;
    m_size = FloatVector2Zero;
    m_requestedPosition = FloatVector2Zero;
    m_requestedSize = FloatVector2Zero;
    m_input.clear();
    m_hasPositionRequest = false;
    m_hasSizeRequest = false;
    m_isFocused = false;
    m_isInputTransparent = false;
    resetInput();
}

bool ego::application::ApplicationGuiViewport::registerSurfaceEvents()
{
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface);

    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    const PlatformSurfaceEventIDs& eventIDs = m_presentation.m_surface->getEventIDs();

    m_surfaceEventCallbackIDs.m_closeRequested = eventController->addInstanceEventCallback<PlatformSurfaceCloseRequestedEvent>(
        eventIDs.m_closeRequested,
        *this,
        &ApplicationGuiViewport::handleSurfaceCloseRequested);
    EGO_CHECK_RETURN_CALL_FALSE(m_surfaceEventCallbackIDs.m_closeRequested != InvalidInstancedEventCallbackID, unregisterSurfaceEvents());

    m_surfaceEventCallbackIDs.m_activation = eventController->addInstanceEventCallback<PlatformSurfaceActivationEvent>(
        eventIDs.m_activation,
        *this,
        &ApplicationGuiViewport::handleSurfaceActivation);
    EGO_CHECK_RETURN_CALL_FALSE(m_surfaceEventCallbackIDs.m_activation != InvalidInstancedEventCallbackID, unregisterSurfaceEvents());

    m_surfaceEventCallbackIDs.m_pointerCaptureLost = eventController->addInstanceEventCallback<PlatformSurfacePointerCaptureLostEvent>(
        eventIDs.m_pointerCaptureLost,
        *this,
        &ApplicationGuiViewport::handleSurfacePointerCaptureLost);
    EGO_CHECK_RETURN_CALL_FALSE(m_surfaceEventCallbackIDs.m_pointerCaptureLost != InvalidInstancedEventCallbackID, unregisterSurfaceEvents());

    m_surfaceEventCallbackIDs.m_keyboardInput = eventController->addInstanceEventCallback<PlatformSurfaceKeyboardInputEvent>(
        eventIDs.m_keyboardInput,
        *this,
        &ApplicationGuiViewport::handleSurfaceKeyboardInput);
    EGO_CHECK_RETURN_CALL_FALSE(m_surfaceEventCallbackIDs.m_keyboardInput != InvalidInstancedEventCallbackID, unregisterSurfaceEvents());

    m_surfaceEventCallbackIDs.m_textInput =
        eventController->addInstanceEventCallback<PlatformSurfaceTextInputEvent>(eventIDs.m_textInput, *this, &ApplicationGuiViewport::handleSurfaceTextInput);
    EGO_CHECK_RETURN_CALL_FALSE(m_surfaceEventCallbackIDs.m_textInput != InvalidInstancedEventCallbackID, unregisterSurfaceEvents());

    return true;
}

void ego::application::ApplicationGuiViewport::unregisterSurfaceEvents()
{
    if (!m_presentation.m_surface)
    {
        m_surfaceEventCallbackIDs = SurfaceEventCallbackIDs();
        return;
    }

    const EventControllerPointer eventController = GetEventControllerPointer();
    if (eventController)
    {
        eventController->removeInstancedEventDispatcher(m_surfaceEventCallbackIDs.m_textInput);
        eventController->removeInstancedEventDispatcher(m_surfaceEventCallbackIDs.m_keyboardInput);
        eventController->removeInstancedEventDispatcher(m_surfaceEventCallbackIDs.m_pointerCaptureLost);
        eventController->removeInstancedEventDispatcher(m_surfaceEventCallbackIDs.m_activation);
        eventController->removeInstancedEventDispatcher(m_surfaceEventCallbackIDs.m_closeRequested);
    }

    m_surfaceEventCallbackIDs = SurfaceEventCallbackIDs();
}

ego::gui::ViewportState ego::application::ApplicationGuiViewport::getState() const
{
    gui::ViewportState state;
    state.m_status = m_status;
    state.m_position = m_position;
    state.m_size = m_size;
    state.m_graphicPresenter = m_presentation.m_graphicPresenter;
    state.m_isFocused = m_isFocused;
    state.m_isInputTransparent = m_isInputTransparent;

    return state;
}

ego::gui::ViewportUpdate ego::application::ApplicationGuiViewport::poll()
{
    const FloatVector2 previousPosition = m_position;
    const FloatVector2 previousSize = m_size;
    updateBounds();

    if (m_status == gui::ViewportUpdateStatus::Alive && !m_presentation.m_graphicPresenter)
    {
        m_status = gui::ViewportUpdateStatus::Lost;
    }

    gui::ViewportUpdate update;
    update.m_state = getState();
    update.m_positionChanged = !AreEqual(previousPosition, m_position);
    update.m_sizeChanged = !AreEqual(previousSize, m_size);
    if (m_hasPositionRequest && AreEqual(m_position, m_requestedPosition))
    {
        update.m_positionChanged = false;
        m_hasPositionRequest = false;
    }
    else if (m_hasPositionRequest && update.m_positionChanged)
    {
        m_hasPositionRequest = false;
    }

    if (m_hasSizeRequest && AreEqual(m_size, m_requestedSize))
    {
        update.m_sizeChanged = false;
        m_hasSizeRequest = false;
    }
    else if (m_hasSizeRequest && update.m_sizeChanged)
    {
        m_hasSizeRequest = false;
    }

    update.m_input.insert(update.m_input.end(), std::make_move_iterator(m_input.begin()), std::make_move_iterator(m_input.end()));
    m_input.clear();

    return update;
}

bool ego::application::ApplicationGuiViewport::show(bool _activate)
{
    return m_presentation.m_surface && m_presentation.m_surface->show(_activate);
}

bool ego::application::ApplicationGuiViewport::setPosition(FloatVector2& _position)
{
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface);

    const SurfacePoint position(ToSurfacePointValue(_position.m_x), ToSurfacePointValue(_position.m_y));
    if (!m_presentation.m_surface->setPosition(position))
    {
        return false;
    }

    m_requestedPosition = FloatVector2(static_cast<float>(position.m_x), static_cast<float>(position.m_y));
    m_hasPositionRequest = !AreEqual(m_position, m_requestedPosition);
    m_position = m_requestedPosition;
    _position = m_requestedPosition;

    return true;
}

bool ego::application::ApplicationGuiViewport::setSize(FloatVector2& _size)
{
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface);

    const SurfaceSize size(ToSurfaceSizeValue(_size.m_x), ToSurfaceSizeValue(_size.m_y));
    EGO_CHECK_RETURN_FALSE(size.m_x != 0 && size.m_y != 0);
    if (!m_presentation.m_surface->setSize(size))
    {
        return false;
    }

    m_requestedSize = FloatVector2(static_cast<float>(size.m_x), static_cast<float>(size.m_y));
    m_hasSizeRequest = !AreEqual(m_size, m_requestedSize);
    m_size = m_requestedSize;
    _size = m_requestedSize;

    return true;
}

bool ego::application::ApplicationGuiViewport::setInputTransparent(bool _isTransparent)
{
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface);
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface->setInputTransparent(_isTransparent));

    m_isInputTransparent = _isTransparent;

    return true;
}

void ego::application::ApplicationGuiViewport::setFocused(bool _isFocused)
{
    m_isFocused = _isFocused;
}

const ego::application::Presentation& ego::application::ApplicationGuiViewport::getPresentation() const
{
    return m_presentation;
}

void ego::application::ApplicationGuiViewport::handleSurfaceCloseRequested(const PlatformSurfaceCloseRequestedEvent& _event)
{
    EGO_CHECK_RETURN(m_presentation.m_surface && &_event.m_surface == m_presentation.m_surface.get());

    m_status = gui::ViewportUpdateStatus::CloseRequested;
    resetInput();

    _event.handle();
}

void ego::application::ApplicationGuiViewport::handleSurfaceActivation(const PlatformSurfaceActivationEvent& _event)
{
    EGO_CHECK_RETURN(m_presentation.m_surface && &_event.m_surface == m_presentation.m_surface.get());

    setFocused(_event.m_isActive);
    if (_event.m_isActive)
    {
        return;
    }

    resetInput();
    m_input.push_back(MakePointer<gui::ViewportDeactivatedEvent>());
}

void ego::application::ApplicationGuiViewport::handleSurfacePointerCaptureLost(const PlatformSurfacePointerCaptureLostEvent& _event)
{
    EGO_CHECK_RETURN(m_presentation.m_surface && &_event.m_surface == m_presentation.m_surface.get());

    m_pressedMouseButtons = 0;
    m_input.push_back(MakePointer<gui::PointerCaptureLostEvent>());
}

void ego::application::ApplicationGuiViewport::handleSurfaceKeyboardInput(const PlatformSurfaceKeyboardInputEvent& _event)
{
    EGO_CHECK_RETURN(m_presentation.m_surface && &_event.m_surface == m_presentation.m_surface.get());

    updateModifiers(_event.m_input);
    if (_event.m_input.m_key == KeyboardInputKey::Undefined)
    {
        return;
    }

    gui::KeyEvent event;
    event.m_key = _event.m_input.m_key;
    event.m_action = _event.m_input.m_action;
    event.m_modifiers = m_modifiers;
    m_input.push_back(MakePointer<gui::KeyEvent>(event));
}

void ego::application::ApplicationGuiViewport::handleSurfaceTextInput(const PlatformSurfaceTextInputEvent& _event)
{
    EGO_CHECK_RETURN(m_presentation.m_surface && &_event.m_surface == m_presentation.m_surface.get());
    EGO_CHECK_RETURN(_event.m_input.m_codepoint != 0);

    gui::TextInputEvent event;
    event.m_codepoint = _event.m_input.m_codepoint;
    event.m_modifiers = m_modifiers;
    m_input.push_back(MakePointer<gui::TextInputEvent>(event));
}

void ego::application::ApplicationGuiViewport::updateBounds()
{
    if (m_status != gui::ViewportUpdateStatus::Alive || !m_presentation.m_surface)
    {
        return;
    }

    SurfacePoint surfacePosition;
    if (m_presentation.m_surface->mapToScreen(DefaultSurfacePoint, surfacePosition))
    {
        m_position = FloatVector2(static_cast<float>(surfacePosition.m_x), static_cast<float>(surfacePosition.m_y));
    }

    const SurfaceSize& surfaceSize = m_presentation.m_surface->getSize();
    m_size = FloatVector2(static_cast<float>(surfaceSize.m_x), static_cast<float>(surfaceSize.m_y));
}

void ego::application::ApplicationGuiViewport::resetInput()
{
    m_modifiers = gui::InputModifiers();
    m_pressedKeyboardModifiers = 0;
    m_pressedMouseButtons = 0;
    m_isPointerInsideSurface = false;
}

void ego::application::ApplicationGuiViewport::updateModifiers(const SurfaceKeyboardInput& _input)
{
    const uint8_t modifierMask = GetKeyboardModifierMask(_input.m_key);
    if (modifierMask == 0)
    {
        return;
    }

    const bool isPressed = _input.m_action == InputButtonAction::Pressed;
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
    m_modifiers.m_super = (m_pressedKeyboardModifiers & SuperModifierMask) != 0;
}

bool ego::application::ApplicationGuiViewport::enqueuePointerExit(const FloatVector2& _screenPosition)
{
    if (!m_isPointerInsideSurface)
    {
        return false;
    }

    FloatVector2 position = _screenPosition;
    bool isInsideSurface = false;
    EGO_CHECK_RETURN_FALSE(convertPointerPosition(position, isInsideSurface));

    m_isPointerInsideSurface = false;

    gui::PointerExitEvent event;
    event.m_position = position;
    event.m_screenPosition = _screenPosition;
    event.m_modifiers = m_modifiers;
    m_input.push_back(MakePointer<gui::PointerExitEvent>(event));

    return true;
}

bool ego::application::ApplicationGuiViewport::enqueueMouseButtonInput(gui::MouseButtonEvent _event)
{
    const uint8_t mouseButtonMask = GetMouseButtonMask(_event.m_key);
    if (_event.m_action == InputButtonAction::Released && (m_pressedMouseButtons & mouseButtonMask) == 0)
    {
        return false;
    }

    const InputButtonAction action = _event.m_action;
    const bool inputEnqueued = enqueuePointerInput(std::move(_event));
    if (action == InputButtonAction::Pressed && inputEnqueued)
    {
        m_pressedMouseButtons |= mouseButtonMask;
    }
    else if (action == InputButtonAction::Released)
    {
        m_pressedMouseButtons &= static_cast<uint8_t>(~mouseButtonMask);
    }

    return inputEnqueued;
}

bool ego::application::ApplicationGuiViewport::enqueuePointerInput(gui::PointerMoveEvent _event)
{
    _event.m_screenPosition = _event.m_position;

    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        if (!emitPointerExit)
        {
            return false;
        }

        gui::PointerExitEvent pointerExitEvent;
        pointerExitEvent.m_position = _event.m_position;
        pointerExitEvent.m_screenPosition = _event.m_screenPosition;
        pointerExitEvent.m_modifiers = m_modifiers;
        m_input.push_back(MakePointer<gui::PointerExitEvent>(pointerExitEvent));
        return true;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(MakePointer<gui::PointerMoveEvent>(std::move(_event)));
    return true;
}

bool ego::application::ApplicationGuiViewport::enqueuePointerInput(gui::MouseButtonEvent _event)
{
    _event.m_screenPosition = _event.m_position;

    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        return false;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(MakePointer<gui::MouseButtonEvent>(std::move(_event)));
    return true;
}

bool ego::application::ApplicationGuiViewport::enqueuePointerInput(gui::MouseWheelEvent _event)
{
    _event.m_screenPosition = _event.m_position;

    bool emitPointerExit = false;
    if (!preparePointerInput(_event.m_position, emitPointerExit))
    {
        return false;
    }

    _event.m_modifiers = m_modifiers;
    m_input.push_back(MakePointer<gui::MouseWheelEvent>(std::move(_event)));
    return true;
}

bool ego::application::ApplicationGuiViewport::preparePointerInput(FloatVector2& _position, bool& _emitPointerExit)
{
    _emitPointerExit = false;
    if (m_status != gui::ViewportUpdateStatus::Alive || !m_presentation.m_surface)
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
    if (m_isPointerInsideSurface || hasPressedMouseButton)
    {
        return true;
    }

    _emitPointerExit = pointerExitedSurface;
    return false;
}

bool ego::application::ApplicationGuiViewport::hasPressedMouseButtons() const
{
    return m_pressedMouseButtons != 0;
}

bool ego::application::ApplicationGuiViewport::convertPointerPosition(FloatVector2& _position, bool& _isInsideSurface) const
{
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface);

    const SurfacePoint screenPoint(ToSurfacePointValue(_position.m_x), ToSurfacePointValue(_position.m_y));
    SurfacePoint localPoint;
    EGO_CHECK_RETURN_FALSE(m_presentation.m_surface->mapFromScreen(screenPoint, localPoint));

    _position = FloatVector2(static_cast<float>(localPoint.m_x), static_cast<float>(localPoint.m_y));

    const SurfaceSize& surfaceSize = m_presentation.m_surface->getSize();
    _isInsideSurface = localPoint.m_x >= 0 && localPoint.m_y >= 0 && localPoint.m_x < static_cast<int32_t>(surfaceSize.m_x) &&
                       localPoint.m_y < static_cast<int32_t>(surfaceSize.m_y);
    return true;
}

bool ego::application::ApplicationGuiViewport::AreEqual(const FloatVector2& _first, const FloatVector2& _second)
{
    return _first.m_x == _second.m_x && _first.m_y == _second.m_y;
}

ego::EventControllerPointer ego::application::ApplicationGuiViewport::GetEventControllerPointer()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();

    return eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
}
