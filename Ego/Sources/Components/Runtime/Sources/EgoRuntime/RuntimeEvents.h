#pragma once

#include "EgoCore/Event/Event.h"

namespace ego::runtime
{
    struct RuntimeQuitRequestedEvent final : public Event
    {
        RuntimeQuitRequestedEvent() = default;

        EGO_EVENT(RuntimeQuitRequestedEvent, Event);
    };
} // namespace ego::runtime
