#pragma once

#include <cstdint>

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Platform/Input/InputTypes.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/Reference/Pointer.h"

#include "InputEventSink.h"

namespace ego::gui
{
    struct InputModifiers final
    {
        bool m_shift = false;
        bool m_control = false;
        bool m_alt = false;
        bool m_super = false;
    };

    class InputEvent
    {
    public:
        virtual ~InputEvent();

        virtual void dispatch(InputEventSink& _sink) const = 0;
    };

    EGO_POINTER(InputEvent);
    using InputEventOwner = InputEventPointer;

    struct PointerMoveEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;

        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_screenPosition = FloatVector2Zero;
        InputModifiers m_modifiers;
    };

    struct PointerExitEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;

        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_screenPosition = FloatVector2Zero;
        InputModifiers m_modifiers;
    };

    struct MouseButtonEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;

        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_screenPosition = FloatVector2Zero;
        MouseInputKey m_key = MouseInputKey::ButtonLeft;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;
    };

    struct MouseWheelEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;

        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_screenPosition = FloatVector2Zero;
        float m_wheelDelta = 0.0f;
        InputModifiers m_modifiers;
    };

    struct KeyEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;

        KeyboardInputKey m_key = KeyboardInputKey::Undefined;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;
    };

    struct TextInputEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;

        uint32_t m_codepoint = 0;
        InputModifiers m_modifiers;
    };

    struct PointerCaptureLostEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;
    };

    struct ViewportDeactivatedEvent final : public InputEvent
    {
        void dispatch(InputEventSink& _sink) const override;
    };
} // namespace ego::gui
