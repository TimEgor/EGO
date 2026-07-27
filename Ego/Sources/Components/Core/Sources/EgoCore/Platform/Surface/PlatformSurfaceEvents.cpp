#include "PlatformSurfaceEvents.h"

ego::PlatformSurfaceEvent::PlatformSurfaceEvent(PlatformSurface& _surface)
    : m_surface(_surface)
{
}

ego::PlatformSurfaceCloseRequestedEvent::PlatformSurfaceCloseRequestedEvent(PlatformSurface& _surface)
    : PlatformSurfaceEvent(_surface)
{
}

void ego::PlatformSurfaceCloseRequestedEvent::handle() const
{
    m_isHandled = true;
}

bool ego::PlatformSurfaceCloseRequestedEvent::isHandled() const
{
    return m_isHandled;
}

ego::PlatformSurfaceDestroyingEvent::PlatformSurfaceDestroyingEvent(PlatformSurface& _surface)
    : PlatformSurfaceEvent(_surface)
{
}

ego::PlatformSurfaceActivationEvent::PlatformSurfaceActivationEvent(PlatformSurface& _surface, bool _isActive)
    : PlatformSurfaceEvent(_surface),
      m_isActive(_isActive)
{
}

ego::PlatformSurfacePointerCaptureLostEvent::PlatformSurfacePointerCaptureLostEvent(PlatformSurface& _surface)
    : PlatformSurfaceEvent(_surface)
{
}

ego::PlatformSurfaceSizeChangedEvent::PlatformSurfaceSizeChangedEvent(PlatformSurface& _surface, const SurfaceSize& _previousSize)
    : PlatformSurfaceEvent(_surface),
      m_previousSize(_previousSize)
{
}

ego::PlatformSurfaceKeyboardInputEvent::PlatformSurfaceKeyboardInputEvent(PlatformSurface& _surface, const SurfaceKeyboardInput& _input)
    : PlatformSurfaceEvent(_surface),
      m_input(_input)
{
}

ego::PlatformSurfaceTextInputEvent::PlatformSurfaceTextInputEvent(PlatformSurface& _surface, const SurfaceTextInput& _input)
    : PlatformSurfaceEvent(_surface),
      m_input(_input)
{
}
