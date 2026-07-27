#pragma once

#include <cstdint>
#include <memory>

#include "EgoCore/Platform/Input/InputTypes.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    struct KeyEvent;
    struct MouseButtonEvent;
    struct MouseWheelEvent;
    struct PointerCaptureLostEvent;
    struct PointerExitEvent;
    struct PointerMoveEvent;
    struct TextInputEvent;
    struct ViewportDeactivatedEvent;

    struct InputModifiers final
    {
        bool m_shift = false;
        bool m_control = false;
        bool m_alt = false;
    };

    enum class InputReply
    {
        Unhandled,
        Handled,
        Capture,
        Focus,
        FocusAndCapture,
        ClearFocus
    };

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

    class InputEvent
    {
    public:
        virtual ~InputEvent();

        virtual void dispatch(InputEventSink& _sink) const = 0;
    };

    using InputEventOwner = std::unique_ptr<InputEvent>;

    struct PointerMoveEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        Position m_screenPosition = PositionZero;
        InputModifiers m_modifiers;

        void dispatch(InputEventSink& _sink) const override;
    };

    struct PointerExitEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        Position m_screenPosition = PositionZero;
        InputModifiers m_modifiers;

        void dispatch(InputEventSink& _sink) const override;
    };

    struct MouseButtonEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        Position m_screenPosition = PositionZero;
        MouseInputKey m_key = MouseInputKey::ButtonLeft;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;

        void dispatch(InputEventSink& _sink) const override;
    };

    struct MouseWheelEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        Position m_screenPosition = PositionZero;
        float m_wheelDelta = 0.0f;
        InputModifiers m_modifiers;

        void dispatch(InputEventSink& _sink) const override;
    };

    struct KeyEvent final : public InputEvent
    {
        KeyboardInputKey m_key = KeyboardInputKey::Undefined;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;

        void dispatch(InputEventSink& _sink) const override;
    };

    struct TextInputEvent final : public InputEvent
    {
        uint32_t m_codepoint = 0;
        InputModifiers m_modifiers;

        void dispatch(InputEventSink& _sink) const override;
    };

    struct PointerCaptureLostEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;
    };

    struct ViewportDeactivatedEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;
    };

    enum class FocusChange
    {
        Gained,
        Lost
    };
} // namespace ego::gui
