#pragma once

#include "EgoCore/Event/EventController.h"
#include "EgoCore/Subsystem/Subsystem.h"

namespace ego
{
    class EventSubsystem final : public subsystem::Subsystem
    {
    public:
        EventSubsystem() = default;
        ~EventSubsystem() override;

        bool init();

        EventControllerPointer getEventControllerPointer() const;
        EventController& getEventController() const;

        EGO_SUBSYSTEM(EventSubsystem, subsystem::Subsystem);

    private:
        void release();

        EventControllerPointer m_eventController = nullptr;
    };

    EGO_POINTER(EventSubsystem);

    EventSubsystemPointer GetEventSubsystemPointer();
    EventSubsystem& GetEventSubsystem();
} // namespace ego
