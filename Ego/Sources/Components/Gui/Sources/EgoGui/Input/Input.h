#pragma once

#include <cstdint>

#include "EgoCore/Platform/Input/InputTypes.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class InputConsumer;

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
        Focus,
        FocusAndCapture,
        ClearFocus
    };

    class InputEvent;
    EGO_POINTER(InputEvent);

    class InputEvent
    {
    public:
        virtual ~InputEvent();

        virtual void sendTo(InputConsumer& _consumer) const = 0;
    };

    struct PointerMoveEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        InputModifiers m_modifiers;

        void sendTo(InputConsumer& _consumer) const override;
    };

    struct PointerExitEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        InputModifiers m_modifiers;

        void sendTo(InputConsumer& _consumer) const override;
    };

    struct MouseButtonEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        MouseInputKey m_key = MouseInputKey::ButtonLeft;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;

        void sendTo(InputConsumer& _consumer) const override;
    };

    struct MouseWheelEvent final : public InputEvent
    {
        Position m_position = PositionZero;
        float m_wheelDelta = 0.0f;
        InputModifiers m_modifiers;

        void sendTo(InputConsumer& _consumer) const override;
    };

    struct KeyEvent final : public InputEvent
    {
        KeyboardInputKey m_key = KeyboardInputKey::Undefined;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;

        void sendTo(InputConsumer& _consumer) const override;
    };

    struct TextInputEvent final : public InputEvent
    {
        uint32_t m_codepoint = 0;
        InputModifiers m_modifiers;

        void sendTo(InputConsumer& _consumer) const override;
    };

    enum class FocusChange
    {
        Gained,
        Lost
    };

    struct ViewportDeactivatedEvent final : public InputEvent
    {
        void sendTo(InputConsumer& _consumer) const override;
    };

    class InputConsumer
    {
    public:
        virtual ~InputConsumer();

        virtual void onPointerMove(const PointerMoveEvent& _event) = 0;
        virtual void onPointerExit(const PointerExitEvent& _event) = 0;
        virtual void onMouseButton(const MouseButtonEvent& _event) = 0;
        virtual void onMouseWheel(const MouseWheelEvent& _event) = 0;
        virtual void onKey(const KeyEvent& _event) = 0;
        virtual void onTextInput(const TextInputEvent& _event) = 0;
        virtual void onViewportDeactivated(const ViewportDeactivatedEvent& _event) = 0;
    };
} // namespace ego::gui
