#pragma once

#include "EgoCore/Event/Event.h"

namespace ego::application
{
    struct ApplicationQuitRequestedEvent final : public Event
    {
        ApplicationQuitRequestedEvent() = default;

        EGO_EVENT(ApplicationQuitRequestedEvent, Event);
    };
} // namespace ego::application
