#include "Window.h"

#include "EgoEngine/Engine.h"

#include "WindowEvents.h"

ego::InstancedEventID ego::Window::getSizeEventID() const
{
    return m_sizeEventID;
}

bool ego::Window::initWindowInstancedEvents()
{
	EventController& eventController = engine::GetEngine().getEventController();

	m_sizeEventID = eventController.registerInstancedEvent<WindowSizeEvent>();
	EGO_CHECK_RETURN_FALSE(m_sizeEventID != InvalidInstancedEventID);

	return true;
}

void ego::Window::releaseWindowInstancedEvents()
{
	EventController& eventController = engine::GetEngine().getEventController();

	if (m_sizeEventID != InvalidInstancedEventID)
	{
		eventController.unregisterInstancedEvent(m_sizeEventID);
		m_sizeEventID = InvalidInstancedEventID;
	}
}
