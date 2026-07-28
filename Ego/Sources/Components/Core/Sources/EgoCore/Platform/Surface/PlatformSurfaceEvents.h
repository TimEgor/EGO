#pragma once

#include "EgoCore/Event/EventController.h"

#include "PlatformSurfaceTypes.h"

namespace ego
{
    class PlatformSurface;

    struct PlatformSurfaceEventIDs final
    {
        InstancedEventID m_closeRequested = InvalidInstancedEventID;
        InstancedEventID m_activation = InvalidInstancedEventID;
        InstancedEventID m_pointerCaptureLost = InvalidInstancedEventID;
        InstancedEventID m_sizeChanged = InvalidInstancedEventID;
        InstancedEventID m_keyboardInput = InvalidInstancedEventID;
        InstancedEventID m_textInput = InvalidInstancedEventID;
    };

    struct PlatformSurfaceEvent : public Event
    {
        PlatformSurface& m_surface;

        explicit PlatformSurfaceEvent(PlatformSurface& _surface);

        EGO_EVENT(PlatformSurfaceEvent, Event);
    };

    struct PlatformSurfaceCloseRequestedEvent final : public PlatformSurfaceEvent
    {
        explicit PlatformSurfaceCloseRequestedEvent(PlatformSurface& _surface);

        void handle() const;
        bool isHandled() const;

        EGO_EVENT(PlatformSurfaceCloseRequestedEvent, PlatformSurfaceEvent);

    private:
        mutable bool m_isHandled = false;
    };

    struct PlatformSurfaceActivationEvent final : public PlatformSurfaceEvent
    {
        bool m_isActive = false;

        PlatformSurfaceActivationEvent(PlatformSurface& _surface, bool _isActive);

        EGO_EVENT(PlatformSurfaceActivationEvent, PlatformSurfaceEvent);
    };

    struct PlatformSurfacePointerCaptureLostEvent final : public PlatformSurfaceEvent
    {
        explicit PlatformSurfacePointerCaptureLostEvent(PlatformSurface& _surface);

        EGO_EVENT(PlatformSurfacePointerCaptureLostEvent, PlatformSurfaceEvent);
    };

    struct PlatformSurfaceSizeChangedEvent final : public PlatformSurfaceEvent
    {
        SurfaceSize m_previousSize = DefaultSurfaceSize;

        PlatformSurfaceSizeChangedEvent(PlatformSurface& _surface, const SurfaceSize& _previousSize);

        EGO_EVENT(PlatformSurfaceSizeChangedEvent, PlatformSurfaceEvent);
    };

    struct PlatformSurfaceKeyboardInputEvent final : public PlatformSurfaceEvent
    {
        SurfaceKeyboardInput m_input;

        PlatformSurfaceKeyboardInputEvent(PlatformSurface& _surface, const SurfaceKeyboardInput& _input);

        EGO_EVENT(PlatformSurfaceKeyboardInputEvent, PlatformSurfaceEvent);
    };

    struct PlatformSurfaceTextInputEvent final : public PlatformSurfaceEvent
    {
        SurfaceTextInput m_input;

        PlatformSurfaceTextInputEvent(PlatformSurface& _surface, const SurfaceTextInput& _input);

        EGO_EVENT(PlatformSurfaceTextInputEvent, PlatformSurfaceEvent);
    };
} // namespace ego
