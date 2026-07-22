#pragma once

#include <cstdint>

#include "EgoCore/Platform/Input/InputTypes.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"

#include "EgoEvent/Event.h"

#include "EgoGraphicHardware/PresentationSurface.h"

namespace ego::application
{
    struct PresentationSurfaceEvent : public Event
    {
        PresentationSurfacePointer m_surface;

        PresentationSurfaceEvent(const PresentationSurfacePointer& _surface)
            : m_surface(_surface)
        {
        }

        EGO_EVENT(PresentationSurfaceEvent, Event);
    };

    struct PresentationSurfaceDestroyingEvent final : public PresentationSurfaceEvent
    {
        PresentationSurfaceDestroyingEvent(const PresentationSurfacePointer& _surface)
            : PresentationSurfaceEvent(_surface)
        {
        }

        EGO_EVENT(PresentationSurfaceDestroyingEvent, PresentationSurfaceEvent);
    };

    struct PresentationSurfaceActivationEvent final : public PresentationSurfaceEvent
    {
        bool m_isActive = false;

        PresentationSurfaceActivationEvent(const PresentationSurfacePointer& _surface, bool _isActive)
            : PresentationSurfaceEvent(_surface),
              m_isActive(_isActive)
        {
        }

        EGO_EVENT(PresentationSurfaceActivationEvent, PresentationSurfaceEvent);
    };

    struct PresentationSurfaceSizeChangedEvent final : public PresentationSurfaceEvent
    {
        PresentationSurfaceSize m_previousSize = PresentationSurfaceSizeZero;

        PresentationSurfaceSizeChangedEvent(const PresentationSurfacePointer& _surface, const PresentationSurfaceSize& _previousSize)
            : PresentationSurfaceEvent(_surface),
              m_previousSize(_previousSize)
        {
        }

        EGO_EVENT(PresentationSurfaceSizeChangedEvent, PresentationSurfaceEvent);
    };

    struct PresentationSurfaceKeyboardInputEvent final : public PresentationSurfaceEvent
    {
        KeyboardInputKey m_key = KeyboardInputKey::Undefined;
        bool m_isRepeat = false;
        InputButtonAction m_action = InputButtonAction::Pressed;

        PresentationSurfaceKeyboardInputEvent(const PresentationSurfacePointer& _surface, KeyboardInputKey _key, bool _isRepeat, InputButtonAction _action)
            : PresentationSurfaceEvent(_surface),
              m_key(_key),
              m_isRepeat(_isRepeat),
              m_action(_action)
        {
        }

        EGO_EVENT(PresentationSurfaceKeyboardInputEvent, PresentationSurfaceEvent);
    };

    struct PresentationSurfaceTextInputEvent final : public PresentationSurfaceEvent
    {
        uint32_t m_codepoint = 0;

        PresentationSurfaceTextInputEvent(const PresentationSurfacePointer& _surface, uint32_t _codepoint)
            : PresentationSurfaceEvent(_surface),
              m_codepoint(_codepoint)
        {
        }

        EGO_EVENT(PresentationSurfaceTextInputEvent, PresentationSurfaceEvent);
    };
} // namespace ego::application
