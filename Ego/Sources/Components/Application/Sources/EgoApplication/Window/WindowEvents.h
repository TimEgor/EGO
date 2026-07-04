#pragma once

#include "EgoRuntime/Event/Event.h"

#include "Window.h"

namespace ego
{
    struct WindowSystemQuitRequestedEvent final : public Event
    {
        WindowSystemQuitRequestedEvent() = default;

        EGO_EVENT(WindowSystemQuitRequestedEvent, Event);
    };

    struct WindowEvent : public Event
    {
        WindowPointer m_window;

        WindowEvent(const WindowPointer& _window)
            : m_window(_window)
        {
        }

        EGO_EVENT(WindowEvent, Event);
    };

    struct WindowDestroyingEvent final : public WindowEvent
    {
        WindowDestroyingEvent(const WindowPointer& _window)
            : WindowEvent(_window)
        {
        }

        EGO_EVENT(WindowDestroyingEvent, WindowEvent);
    };

    struct WindowActivationEvent final : public WindowEvent
    {
        bool m_isActive = false;

        WindowActivationEvent(const WindowPointer& _window, bool _isActive)
            : WindowEvent(_window),
              m_isActive(_isActive)
        {
        }

        EGO_EVENT(WindowActivationEvent, WindowEvent);
    };

    struct WindowSizeEvent final : public WindowEvent
    {
        WindowSize m_prevWindowSize;

        WindowSizeEvent(const WindowPointer& _window, const WindowSize& _prevSize)
            : WindowEvent(_window),
              m_prevWindowSize(_prevSize)
        {
        }

        EGO_EVENT(WindowSizeEvent, WindowEvent);
    };
} // namespace ego
