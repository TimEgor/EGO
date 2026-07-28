#include "PlatformSurface.h"

#include "EgoCore/Event/EventSubsystem.h"
#include "EgoCore/UtilsMacros.h"

ego::PlatformSurface::~PlatformSurface()
{
    releaseEvents();
}

const ego::PlatformSurfaceEventIDs& ego::PlatformSurface::getEventIDs() const
{
    return m_eventIDs;
}

bool ego::PlatformSurface::initEvents()
{
    EGO_CHECK_RETURN_FALSE(m_eventIDs.m_closeRequested == InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(m_eventIDs.m_activation == InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(m_eventIDs.m_pointerCaptureLost == InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(m_eventIDs.m_sizeChanged == InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(m_eventIDs.m_keyboardInput == InvalidInstancedEventID);
    EGO_CHECK_RETURN_FALSE(m_eventIDs.m_textInput == InvalidInstancedEventID);

    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(eventController);

    m_eventIDs.m_closeRequested = eventController->registerInstancedEvent<PlatformSurfaceCloseRequestedEvent>();
    EGO_CHECK_RETURN_CALL_FALSE(m_eventIDs.m_closeRequested != InvalidInstancedEventID, releaseEvents());

    m_eventIDs.m_activation = eventController->registerInstancedEvent<PlatformSurfaceActivationEvent>();
    EGO_CHECK_RETURN_CALL_FALSE(m_eventIDs.m_activation != InvalidInstancedEventID, releaseEvents());

    m_eventIDs.m_pointerCaptureLost = eventController->registerInstancedEvent<PlatformSurfacePointerCaptureLostEvent>();
    EGO_CHECK_RETURN_CALL_FALSE(m_eventIDs.m_pointerCaptureLost != InvalidInstancedEventID, releaseEvents());

    m_eventIDs.m_sizeChanged = eventController->registerInstancedEvent<PlatformSurfaceSizeChangedEvent>();
    EGO_CHECK_RETURN_CALL_FALSE(m_eventIDs.m_sizeChanged != InvalidInstancedEventID, releaseEvents());

    m_eventIDs.m_keyboardInput = eventController->registerInstancedEvent<PlatformSurfaceKeyboardInputEvent>();
    EGO_CHECK_RETURN_CALL_FALSE(m_eventIDs.m_keyboardInput != InvalidInstancedEventID, releaseEvents());

    m_eventIDs.m_textInput = eventController->registerInstancedEvent<PlatformSurfaceTextInputEvent>();
    EGO_CHECK_RETURN_CALL_FALSE(m_eventIDs.m_textInput != InvalidInstancedEventID, releaseEvents());

    return true;
}

void ego::PlatformSurface::releaseEvents()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    if (eventController)
    {
        eventController->unregisterInstancedEvent(m_eventIDs.m_textInput);
        eventController->unregisterInstancedEvent(m_eventIDs.m_keyboardInput);
        eventController->unregisterInstancedEvent(m_eventIDs.m_sizeChanged);
        eventController->unregisterInstancedEvent(m_eventIDs.m_pointerCaptureLost);
        eventController->unregisterInstancedEvent(m_eventIDs.m_activation);
        eventController->unregisterInstancedEvent(m_eventIDs.m_closeRequested);
    }

    m_eventIDs = PlatformSurfaceEventIDs();
}

bool ego::PlatformSurface::notifyCloseRequested()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(eventController && m_eventIDs.m_closeRequested != InvalidInstancedEventID);

    const PlatformSurfaceCloseRequestedEvent event(*this);
    EGO_CHECK_RETURN_FALSE(eventController->emitInstancedEvent(m_eventIDs.m_closeRequested, event));

    return event.isHandled();
}

void ego::PlatformSurface::notifyActivation(bool _isActive)
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN(eventController && m_eventIDs.m_activation != InvalidInstancedEventID);

    const PlatformSurfaceActivationEvent event(*this, _isActive);
    eventController->emitInstancedEvent(m_eventIDs.m_activation, event);
}

void ego::PlatformSurface::notifyPointerCaptureLost()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN(eventController && m_eventIDs.m_pointerCaptureLost != InvalidInstancedEventID);

    const PlatformSurfacePointerCaptureLostEvent event(*this);
    eventController->emitInstancedEvent(m_eventIDs.m_pointerCaptureLost, event);
}

void ego::PlatformSurface::notifySizeChanged(const SurfaceSize& _previousSize)
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN(eventController && m_eventIDs.m_sizeChanged != InvalidInstancedEventID);

    const PlatformSurfaceSizeChangedEvent event(*this, _previousSize);
    eventController->emitInstancedEvent(m_eventIDs.m_sizeChanged, event);
}

void ego::PlatformSurface::notifyKeyboardInput(const SurfaceKeyboardInput& _input)
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN(eventController && m_eventIDs.m_keyboardInput != InvalidInstancedEventID);

    const PlatformSurfaceKeyboardInputEvent event(*this, _input);
    eventController->emitInstancedEvent(m_eventIDs.m_keyboardInput, event);
}

void ego::PlatformSurface::notifyTextInput(const SurfaceTextInput& _input)
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN(eventController && m_eventIDs.m_textInput != InvalidInstancedEventID);

    const PlatformSurfaceTextInputEvent event(*this, _input);
    eventController->emitInstancedEvent(m_eventIDs.m_textInput, event);
}
