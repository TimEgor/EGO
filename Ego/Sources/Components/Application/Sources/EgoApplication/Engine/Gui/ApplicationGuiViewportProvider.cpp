#include "ApplicationGuiViewportProvider.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

#include "EgoCore/Event/EventSubsystem.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoInput/InputEvents.h"

#include "EgoApplication/ApplicationSubsystem.h"

#include "ApplicationGuiViewport.h"

namespace
{
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

    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }

    ego::FloatVector2 GetMousePosition(const ego::InputDevice& _device)
    {
        return ego::FloatVector2(_device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)), _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)));
    }

    bool HasMousePositionChanged(const ego::InputDevice& _device)
    {
        return _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) != _device.getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) ||
               _device.getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)) != _device.getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisY));
    }

    void RemoveEventCallback(const ego::EventControllerPointer& _controller, ego::EventCallbackID& _callbackID)
    {
        if (_controller && _callbackID != ego::InvalidEventCallbackID)
        {
            _controller->removeEventCallback(_callbackID);
        }
        _callbackID = ego::InvalidEventCallbackID;
    }
} // namespace

ego::application::ApplicationGuiViewportProvider::~ApplicationGuiViewportProvider()
{
    release();
}

bool ego::application::ApplicationGuiViewportProvider::init(const Presentation& _primaryPresentation)
{
    EGO_CHECK_INITIALIZATION(
        !m_primaryPresentation.m_surface &&
        m_viewports.empty() &&
        m_modalViewportStack.empty() &&
        m_retiringSurfaces.empty() &&
        m_releasableSurfaces.empty() &&
        m_primaryViewportID == gui::InvalidViewportID
    );
    EGO_CHECK_INITIALIZATION(_primaryPresentation.m_surface && _primaryPresentation.m_graphicPresenter);

    m_primaryPresentation = _primaryPresentation;
    EGO_CHECK_RETURN_CALL_FALSE(registerInputEvents(), release());

    return true;
}

void ego::application::ApplicationGuiViewportProvider::release()
{
    unregisterInputEvents();
    m_pointerViewportID = gui::InvalidViewportID;

    for (ViewportMap::value_type& viewportEntry : m_viewports)
    {
        releaseViewport(viewportEntry.second);
    }

    m_viewports.clear();
    m_modalViewportStack.clear();
    releaseSurfaces(m_retiringSurfaces);
    releaseSurfaces(m_releasableSurfaces);
    m_primaryViewportID = gui::InvalidViewportID;
    m_primaryPresentation = Presentation();
}

bool ego::application::ApplicationGuiViewportProvider::createViewport(const gui::ViewportCreateRequest& _request)
{
    EGO_CHECK_RETURN_FALSE(_request.m_id != gui::InvalidViewportID);
    EGO_CHECK_RETURN_FALSE(m_primaryPresentation.m_surface);
    EGO_CHECK_RETURN_FALSE(!m_viewports.contains(_request.m_id));
    EGO_CHECK_RETURN_FALSE(_request.m_role != gui::ViewportRole::Primary || m_primaryViewportID == gui::InvalidViewportID);
    EGO_CHECK_RETURN_FALSE(_request.m_parentID == gui::InvalidViewportID || m_viewports.contains(_request.m_parentID));
    EGO_CHECK_RETURN_FALSE(!_request.m_isModal || _request.m_role == gui::ViewportRole::Secondary);

    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    EGO_CHECK_RETURN_FALSE(presenterProvider);

    const bool isSecondary = _request.m_role == gui::ViewportRole::Secondary;
    const Presentation presentation =
        isSecondary ? presenterProvider->createPresentation(createViewportPresentationDesc(_request)) : m_primaryPresentation;
    if (!presentation.m_surface || !presentation.m_graphicPresenter)
    {
        if (isSecondary && presentation.m_surface)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }

        return false;
    }

    ViewportPointer viewport = MakePointer<ApplicationGuiViewport>();
    if (!viewport || !viewport->init(presentation))
    {
        if (viewport)
        {
            viewport->release();
            viewport = nullptr;
        }

        if (isSecondary)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }

        return false;
    }

    const std::pair<ViewportMap::iterator, bool> insertResult = m_viewports.emplace(_request.m_id, viewport);
    if (!insertResult.second)
    {
        releaseViewport(viewport);

        return false;
    }

    if (!isSecondary)
    {
        m_primaryViewportID = _request.m_id;
        setFocusedViewport(_request.m_id);
    }

    if (_request.m_isModal)
    {
        m_modalViewportStack.push_back(_request.m_id);
    }

    if (!updateViewportInputState())
    {
        removeModalViewport(_request.m_id);
        if (!isSecondary)
        {
            m_primaryViewportID = gui::InvalidViewportID;
        }

        m_viewports.erase(insertResult.first);
        releaseViewport(viewport);

        const bool isInputRestored = updateViewportInputState();
        EGO_ASSERT(isInputRestored);

        return false;
    }

    return true;
}

void ego::application::ApplicationGuiViewportProvider::destroyViewport(gui::ViewportID _viewportID)
{
    ViewportMap::iterator viewportIt = m_viewports.find(_viewportID);
    if (viewportIt == m_viewports.end())
    {
        return;
    }

    if (m_pointerViewportID == _viewportID)
    {
        m_pointerViewportID = gui::InvalidViewportID;
    }

    removeModalViewport(_viewportID);

    if (_viewportID == m_primaryViewportID)
    {
        m_primaryViewportID = gui::InvalidViewportID;
        releaseViewport(viewportIt->second);
    }
    else
    {
        retireViewport(viewportIt->second);
    }

    m_viewports.erase(viewportIt);

    const bool isInputUpdated = updateViewportInputState();
    EGO_ASSERT(isInputUpdated);
}

ego::gui::ViewportState ego::application::ApplicationGuiViewportProvider::getViewportState(gui::ViewportID _viewportID) const
{
    const ViewportPointer viewport = findViewport(_viewportID);
    return viewport ? viewport->getState() : gui::ViewportState();
}

ego::gui::ViewportUpdate ego::application::ApplicationGuiViewportProvider::pollViewport(gui::ViewportID _viewportID)
{
    if (_viewportID == m_primaryViewportID)
    {
        advanceViewportRetirement();
    }

    const ViewportMap::iterator viewportIt = m_viewports.find(_viewportID);
    if (viewportIt == m_viewports.end() || !viewportIt->second)
    {
        return gui::ViewportUpdate();
    }

    return viewportIt->second->poll();
}

bool ego::application::ApplicationGuiViewportProvider::showViewport(gui::ViewportID _viewportID, bool _activate)
{
    const ViewportPointer viewport = findViewport(_viewportID);
    EGO_CHECK_RETURN_FALSE(viewport);
    EGO_CHECK_RETURN_FALSE(viewport->show(_activate));

    if (_activate)
    {
        setFocusedViewport(_viewportID);
    }

    return true;
}

bool ego::application::ApplicationGuiViewportProvider::setViewportPosition(gui::ViewportID _viewportID, FloatVector2& _position)
{
    const ViewportPointer viewport = findViewport(_viewportID);
    EGO_CHECK_RETURN_FALSE(viewport);

    return viewport->setPosition(_position);
}

bool ego::application::ApplicationGuiViewportProvider::setViewportSize(gui::ViewportID _viewportID, FloatVector2& _size)
{
    const ViewportPointer viewport = findViewport(_viewportID);
    EGO_CHECK_RETURN_FALSE(viewport);

    return viewport->setSize(_size);
}

bool ego::application::ApplicationGuiViewportProvider::setViewportInputTransparent(gui::ViewportID _viewportID, bool _isTransparent)
{
    const ViewportPointer viewport = findViewport(_viewportID);
    EGO_CHECK_RETURN_FALSE(viewport);

    return viewport->setInputTransparent(_isTransparent);
}

bool ego::application::ApplicationGuiViewportProvider::registerInputEvents()
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    unregisterInputEvents();

    m_callbackIDs.m_mouseChanged = eventController->addEventCallback<InputDeviceChangedEvent>(*this, &ApplicationGuiViewportProvider::handleMouseChangedEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseChanged != InvalidEventCallbackID, unregisterInputEvents());

    m_callbackIDs.m_mouseWheel = eventController->addEventCallback<InputKeyChangedEvent>(*this, &ApplicationGuiViewportProvider::handleMouseWheelEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseWheel != InvalidEventCallbackID, unregisterInputEvents());

    m_callbackIDs.m_mouseButtonPressed =
        eventController->addEventCallback<InputButtonPressedEvent>(*this, &ApplicationGuiViewportProvider::handleMouseButtonPressedEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseButtonPressed != InvalidEventCallbackID, unregisterInputEvents());

    m_callbackIDs.m_mouseButtonReleased =
        eventController->addEventCallback<InputButtonReleasedEvent>(*this, &ApplicationGuiViewportProvider::handleMouseButtonReleasedEvent);
    EGO_CHECK_RETURN_CALL_FALSE(m_callbackIDs.m_mouseButtonReleased != InvalidEventCallbackID, unregisterInputEvents());

    return true;
}

void ego::application::ApplicationGuiViewportProvider::unregisterInputEvents()
{
    const EventControllerPointer eventController = GetEventControllerPointer();

    RemoveEventCallback(eventController, m_callbackIDs.m_mouseButtonReleased);
    RemoveEventCallback(eventController, m_callbackIDs.m_mouseButtonPressed);
    RemoveEventCallback(eventController, m_callbackIDs.m_mouseWheel);
    RemoveEventCallback(eventController, m_callbackIDs.m_mouseChanged);
}

void ego::application::ApplicationGuiViewportProvider::handleMouseChangedEvent(const InputDeviceChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || !HasMousePositionChanged(*_event.m_device))
    {
        return;
    }

    const FloatVector2 position = GetMousePosition(*_event.m_device);
    const gui::ViewportID viewportID = findPointerInputViewport(position);
    updatePointerViewport(viewportID, position);

    const ViewportPointer viewport = findViewport(viewportID);
    if (viewport)
    {
        gui::PointerMoveEvent event;
        event.m_position = position;
        viewport->enqueuePointerInput(std::move(event));
    }
}

void ego::application::ApplicationGuiViewportProvider::handleMouseWheelEvent(const InputKeyChangedEvent& _event)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device || _event.m_key != ToInputDeviceKey(MouseInputKey::Wheel))
    {
        return;
    }

    const FloatVector2 position = GetMousePosition(*_event.m_device);
    const gui::ViewportID viewportID = findPointerInputViewport(position);
    updatePointerViewport(viewportID, position);

    const ViewportPointer viewport = findViewport(viewportID);
    if (viewport)
    {
        gui::MouseWheelEvent event;
        event.m_position = position;
        event.m_wheelDelta = _event.m_value - _event.m_previousValue;
        viewport->enqueuePointerInput(std::move(event));
    }
}

void ego::application::ApplicationGuiViewportProvider::handleMouseButtonPressedEvent(const InputKeyEvent& _event)
{
    handleMouseButtonEvent(_event, InputButtonAction::Pressed);
}

void ego::application::ApplicationGuiViewportProvider::handleMouseButtonReleasedEvent(const InputKeyEvent& _event)
{
    handleMouseButtonEvent(_event, InputButtonAction::Released);
}

void ego::application::ApplicationGuiViewportProvider::handleMouseButtonEvent(const InputKeyEvent& _event, InputButtonAction _action)
{
    if (_event.m_deviceType != MouseInputDevice::GetMetaInfoID() || !_event.m_device)
    {
        return;
    }

    if (_event.m_key < ToInputDeviceKey(MouseInputKey::ButtonLeft) || _event.m_key > ToInputDeviceKey(MouseInputKey::ButtonEight))
    {
        return;
    }

    const FloatVector2 position = GetMousePosition(*_event.m_device);
    const gui::ViewportID viewportID = findPointerInputViewport(position);
    updatePointerViewport(viewportID, position);

    const ViewportPointer viewport = findViewport(viewportID);
    if (!viewport)
    {
        return;
    }

    const bool hadPressedMouseButtons = viewport->hasPressedMouseButtons();

    gui::MouseButtonEvent event;
    event.m_position = position;
    event.m_key = static_cast<MouseInputKey>(_event.m_key);
    event.m_action = _action;

    const bool inputEnqueued = viewport->enqueueMouseButtonInput(std::move(event));
    if (_action == InputButtonAction::Pressed && inputEnqueued && !hadPressedMouseButtons)
    {
        setFocusedViewport(viewportID);
        capturePointer(viewport);
    }
    else if (_action == InputButtonAction::Released && !viewport->hasPressedMouseButtons())
    {
        releasePointer(viewport);
    }
}

ego::gui::ViewportID ego::application::ApplicationGuiViewportProvider::findViewportAtScreenPosition(const FloatVector2& _position) const
{
    const SurfacePoint position(ToSurfacePointValue(_position.m_x), ToSurfacePointValue(_position.m_y));
    const PlatformPointer platform = GetPlatformPointer();
    const PlatformSurfacePointer targetSurface = platform ? platform->getSurfaceController().findSurfaceAtPoint(position) : nullptr;
    if (!targetSurface)
    {
        return gui::InvalidViewportID;
    }

    for (const ViewportMap::value_type& viewportEntry : m_viewports)
    {
        const ViewportPointer viewport = viewportEntry.second;
        const PlatformSurfacePointer surface = viewport ? viewport->getPresentation().m_surface : nullptr;
        if (surface.get() == targetSurface.get())
        {
            return viewportEntry.first;
        }
    }

    return gui::InvalidViewportID;
}

void ego::application::ApplicationGuiViewportProvider::retireViewport(ViewportPointer& _viewport)
{
    if (!_viewport)
    {
        return;
    }

    releasePointer(_viewport);

    const PlatformSurfacePointer surface = _viewport->getPresentation().m_surface;
    if (surface)
    {
        surface->setInputTransparent(false);
        surface->hide();
    }

    _viewport->release();
    _viewport = nullptr;

    if (surface)
    {
        m_retiringSurfaces.push_back(surface);
    }
}

void ego::application::ApplicationGuiViewportProvider::advanceViewportRetirement()
{
    releaseSurfaces(m_releasableSurfaces);
    m_releasableSurfaces.swap(m_retiringSurfaces);
}

void ego::application::ApplicationGuiViewportProvider::releaseSurfaces(SurfaceCollection& _surfaces)
{
    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    if (presenterProvider)
    {
        for (PlatformSurfacePointer& surface : _surfaces)
        {
            presenterProvider->destroyPresentation(surface);
        }
    }

    _surfaces.clear();
}

void ego::application::ApplicationGuiViewportProvider::releaseViewport(ViewportPointer& _viewport)
{
    if (!_viewport)
    {
        return;
    }

    releasePointer(_viewport);

    const PlatformSurfacePointer surface = _viewport->getPresentation().m_surface;
    _viewport->release();
    _viewport = nullptr;

    if (!surface || surface.get() == m_primaryPresentation.m_surface.get())
    {
        return;
    }

    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    if (presenterProvider)
    {
        presenterProvider->destroyPresentation(surface);
    }
}

void ego::application::ApplicationGuiViewportProvider::removeModalViewport(gui::ViewportID _viewportID)
{
    std::erase(m_modalViewportStack, _viewportID);
}

bool ego::application::ApplicationGuiViewportProvider::updateViewportInputState()
{
    const gui::ViewportID activeModalViewportID = m_modalViewportStack.empty() ? gui::InvalidViewportID : m_modalViewportStack.back();
    bool isInputUpdated = true;
    for (ViewportMap::value_type& viewportEntry : m_viewports)
    {
        const bool isInputEnabled = activeModalViewportID == gui::InvalidViewportID || viewportEntry.first == activeModalViewportID;
        if (!viewportEntry.second || !viewportEntry.second->setInputEnabled(isInputEnabled))
        {
            isInputUpdated = false;
        }
    }

    return isInputUpdated;
}

ego::application::ApplicationGuiViewportProvider::ViewportPointer ego::application::ApplicationGuiViewportProvider::findViewport(
    gui::ViewportID _viewportID) const
{
    const ViewportMap::const_iterator viewportIt = m_viewports.find(_viewportID);
    return viewportIt != m_viewports.end() ? viewportIt->second : nullptr;
}

ego::gui::ViewportID ego::application::ApplicationGuiViewportProvider::findPointerInputViewport(const FloatVector2& _position) const
{
    for (const ViewportMap::value_type& viewportEntry : m_viewports)
    {
        const ViewportPointer viewport = viewportEntry.second;
        const PlatformSurfacePointer surface = viewport ? viewport->getPresentation().m_surface : nullptr;
        if (surface && surface->hasPointerCapture())
        {
            return viewportEntry.first;
        }
    }

    return findViewportAtScreenPosition(_position);
}

bool ego::application::ApplicationGuiViewportProvider::capturePointer(const ViewportPointer& _viewport)
{
    EGO_CHECK_RETURN_FALSE(_viewport);

    const PlatformSurfacePointer surface = _viewport->getPresentation().m_surface;
    return surface && surface->capturePointer();
}

void ego::application::ApplicationGuiViewportProvider::releasePointer(const ViewportPointer& _viewport)
{
    if (!_viewport)
    {
        return;
    }

    const PlatformSurfacePointer surface = _viewport->getPresentation().m_surface;
    if (surface && surface->hasPointerCapture())
    {
        surface->releasePointer();
    }
}

void ego::application::ApplicationGuiViewportProvider::setFocusedViewport(gui::ViewportID _viewportID)
{
    for (ViewportMap::value_type& viewportEntry : m_viewports)
    {
        if (viewportEntry.second)
        {
            viewportEntry.second->setFocused(viewportEntry.first == _viewportID);
        }
    }
}

void ego::application::ApplicationGuiViewportProvider::updatePointerViewport(gui::ViewportID _viewportID, const FloatVector2& _position)
{
    if (m_pointerViewportID == _viewportID)
    {
        return;
    }

    const ViewportPointer previousViewport = findViewport(m_pointerViewportID);
    if (previousViewport)
    {
        previousViewport->enqueuePointerExit(_position);
    }

    m_pointerViewportID = _viewportID;
}

ego::EventControllerPointer ego::application::ApplicationGuiViewportProvider::GetEventControllerPointer()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    return eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
}

ego::application::PresenterProviderPointer ego::application::ApplicationGuiViewportProvider::GetPresenterProvider()
{
    const ApplicationPointer application = GetApplicationPointer();
    return application ? application->getPresenterProviderPointer() : nullptr;
}

ego::application::PresentationDesc ego::application::ApplicationGuiViewportProvider::createViewportPresentationDesc(
    const gui::ViewportCreateRequest& _request) const
{
    constexpr SurfaceSize DefaultViewportSize(500, 500);

    SurfaceSize viewportSize(ToSurfaceSizeValue(_request.m_size.m_x), ToSurfaceSizeValue(_request.m_size.m_y));
    if (viewportSize.m_x == 0 || viewportSize.m_y == 0)
    {
        viewportSize = DefaultViewportSize;
    }

    const ViewportPointer parentViewport = findViewport(_request.m_parentID);
    const PlatformSurfacePointer ownerSurface = parentViewport ? parentViewport->getPresentation().m_surface : nullptr;

    return PresentationDesc{
        .m_name = _request.m_title,
        .m_size = viewportSize,
        .m_position = SurfacePoint(ToSurfacePointValue(_request.m_position.m_x), ToSurfacePointValue(_request.m_position.m_y)),
        .m_ownerSurface = ownerSurface,
        .m_hasFrame = false
    };
}
