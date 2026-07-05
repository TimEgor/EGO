#pragma once

#include "EgoRuntime/Event/Event.h"

#include "ApplicationWindow.h"

namespace ego::application
{
    struct ApplicationQuitRequestedEvent final : public Event
    {
        ApplicationQuitRequestedEvent() = default;

        EGO_EVENT(ApplicationQuitRequestedEvent, Event);
    };

    struct ApplicationWindowEvent : public Event
    {
        ApplicationWindowPointer m_window;

        ApplicationWindowEvent(const ApplicationWindowPointer& _window)
            : m_window(_window)
        {
        }

        EGO_EVENT(ApplicationWindowEvent, Event);
    };

    struct ApplicationWindowDestroyingEvent final : public ApplicationWindowEvent
    {
        ApplicationWindowDestroyingEvent(const ApplicationWindowPointer& _window)
            : ApplicationWindowEvent(_window)
        {
        }

        EGO_EVENT(ApplicationWindowDestroyingEvent, ApplicationWindowEvent);
    };

    struct ApplicationWindowActivationEvent final : public ApplicationWindowEvent
    {
        bool m_isActive = false;

        ApplicationWindowActivationEvent(const ApplicationWindowPointer& _window, bool _isActive)
            : ApplicationWindowEvent(_window),
              m_isActive(_isActive)
        {
        }

        EGO_EVENT(ApplicationWindowActivationEvent, ApplicationWindowEvent);
    };

    struct ApplicationWindowSizeChangedEvent final : public ApplicationWindowEvent
    {
        WindowSize m_prevWindowSize;

        ApplicationWindowSizeChangedEvent(const ApplicationWindowPointer& _window, const WindowSize& _prevSize)
            : ApplicationWindowEvent(_window),
              m_prevWindowSize(_prevSize)
        {
        }

        EGO_EVENT(ApplicationWindowSizeChangedEvent, ApplicationWindowEvent);
    };
} // namespace ego::application
