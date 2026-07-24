#pragma once

#include <cstdint>
#include <memory>

#include "EgoCore/Platform/Input/InputTypes.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class WidgetUpdateContext;

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

    class InputEvent
    {
    public:
        virtual ~InputEvent();

    private:
        friend class WidgetUpdateContext;

        virtual void update(WidgetUpdateContext& _context) const = 0;
    };

    using InputEventOwner = std::unique_ptr<InputEvent>;

    struct PointerMoveEvent final
        : public InputEvent
    {
        Position m_position = PositionZero;
        InputModifiers m_modifiers;

    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    struct PointerExitEvent final
        : public InputEvent
    {
        Position m_position = PositionZero;
        InputModifiers m_modifiers;

    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    struct MouseButtonEvent final
        : public InputEvent
    {
        Position m_position = PositionZero;
        MouseInputKey m_key = MouseInputKey::ButtonLeft;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;

    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    struct MouseWheelEvent final
        : public InputEvent
    {
        Position m_position = PositionZero;
        float m_wheelDelta = 0.0f;
        InputModifiers m_modifiers;

    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    struct KeyEvent final
        : public InputEvent
    {
        KeyboardInputKey m_key = KeyboardInputKey::Undefined;
        InputButtonAction m_action = InputButtonAction::Pressed;
        InputModifiers m_modifiers;

    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    struct TextInputEvent final
        : public InputEvent
    {
        uint32_t m_codepoint = 0;
        InputModifiers m_modifiers;

    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    struct ViewportDeactivatedEvent final
        : public InputEvent
    {
    private:
        void update(WidgetUpdateContext& _context) const override;
    };

    enum class FocusChange
    {
        Gained,
        Lost
    };
} // namespace ego::gui
