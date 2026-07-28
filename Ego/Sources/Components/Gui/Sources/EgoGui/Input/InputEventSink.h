#pragma once

namespace ego::gui
{
    struct PointerMoveEvent;
    struct PointerExitEvent;
    struct MouseButtonEvent;
    struct MouseWheelEvent;
    struct KeyEvent;
    struct TextInputEvent;
    struct PointerCaptureLostEvent;
    struct ViewportDeactivatedEvent;

    class InputEventSink
    {
    public:
        virtual ~InputEventSink();

        virtual void process(const PointerMoveEvent& _event) = 0;
        virtual void process(const PointerExitEvent& _event) = 0;
        virtual void process(const MouseButtonEvent& _event) = 0;
        virtual void process(const MouseWheelEvent& _event) = 0;
        virtual void process(const KeyEvent& _event) = 0;
        virtual void process(const TextInputEvent& _event) = 0;
        virtual void process(const PointerCaptureLostEvent& _event) = 0;
        virtual void process(const ViewportDeactivatedEvent& _event) = 0;
    };
} // namespace ego::gui
