#include "EgoCore/Event/EventSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

ego::EventSubsystem::~EventSubsystem()
{
    release();
}

bool ego::EventSubsystem::init()
{
    m_eventController = MakePointer<EventController>();
    EGO_CHECK_INITIALIZATION(m_eventController && m_eventController->init());

    return true;
}

void ego::EventSubsystem::onUnregistered()
{
    release();
}

void ego::EventSubsystem::release()
{
    if (m_eventController)
    {
        EGO_ASSERT(m_eventController.getUsingCount() == 1);
        m_eventController->release();
    }

    m_eventController = nullptr;
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
