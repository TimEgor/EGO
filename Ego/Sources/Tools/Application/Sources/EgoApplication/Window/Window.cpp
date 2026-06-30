#include "Window.h"

#include "EgoApplication/Application.h"

#include "WindowEvents.h"

ego::InstancedEventID ego::Window::getSizeEventID() const
{
    return m_sizeEventID;
}

bool ego::Window::initWindowInstancedEvents()
{
    EventController& eventController = application::GetApplication().getEventController();

    m_sizeEventID = eventController.registerInstancedEvent<WindowSizeEvent>();
    EGO_CHECK_RETURN_FALSE(m_sizeEventID != InvalidInstancedEventID);

    return true;
}

void ego::Window::releaseWindowInstancedEvents()
{
    EventController& eventController = application::GetApplication().getEventController();

    if (m_sizeEventID != InvalidInstancedEventID)
    {
        eventController.unregisterInstancedEvent(m_sizeEventID);
        m_sizeEventID = InvalidInstancedEventID;
    }
}
