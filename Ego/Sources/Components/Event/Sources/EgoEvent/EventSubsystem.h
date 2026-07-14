#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

namespace ego
{
    class EventController;

    EGO_POINTER(EventController);

    class EventSubsystem final : public subsystem::Subsystem
    {
    public:
        EventSubsystem() = default;
        ~EventSubsystem() override = default;

        bool init();
        void release() override;

        EventControllerPointer getEventControllerPointer() const;
        EventController& getEventController() const;

        EGO_SUBSYSTEM(EventSubsystem, subsystem::Subsystem);

    private:
        EventControllerPointer m_eventController = nullptr;
    };

    EGO_POINTER(EventSubsystem);

    EventSubsystemPointer GetEventSubsystemPointer();
    EventSubsystem& GetEventSubsystem();
} // namespace ego
