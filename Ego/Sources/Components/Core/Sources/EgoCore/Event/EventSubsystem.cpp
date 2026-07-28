#include "EgoCore/Event/EventSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

bool ego::EventSubsystem::init()
{
    m_eventController = MakePointer<EventController>();
    EGO_CHECK_INITIALIZATION(m_eventController && m_eventController->init());

    return true;
}

void ego::EventSubsystem::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_eventController);
}

ego::EventControllerPointer ego::EventSubsystem::getEventControllerPointer() const
{
    return m_eventController;
}

ego::EventController& ego::EventSubsystem::getEventController() const
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

ego::EventSubsystemPointer ego::GetEventSubsystemPointer()
{
    return subsystem::FindSubsystem<EventSubsystem>();
}

ego::EventSubsystem& ego::GetEventSubsystem()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    EGO_ASSERT(eventSubsystem);

    return *eventSubsystem;
}
