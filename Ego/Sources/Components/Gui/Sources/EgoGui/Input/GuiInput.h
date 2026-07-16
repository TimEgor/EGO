#pragma once

#include <cstdint>

#include "EgoGui/Core/GuiTypes.h"

namespace ego::gui
{
    enum class GuiInputEventType
    {
        Undefined,
        MouseMove,
        PointerLeave,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel,
        KeyDown,
        KeyUp,
        TextInput,
        FocusLost
    };

    enum class GuiMouseButton
    {
        Left,
        Right,
        Middle
    };

    enum class GuiKey
    {
        Undefined,
        Backspace,
        Tab,
        Enter,
        Shift,
        Control,
        Escape,
        End,
        Home,
        Left,
        Right,
        Delete,
        A
    };

    struct GuiInputEvent final
    {
        GuiInputEventType m_type = GuiInputEventType::Undefined;
        GuiPosition m_position = GuiPositionZero;
        GuiMouseButton m_mouseButton = GuiMouseButton::Left;
        float m_wheelDelta = 0.0f;
        GuiKey m_key = GuiKey::Undefined;
        uint32_t m_textCodepoint = 0;
    };

    enum class GuiEventResult
    {
        Unhandled,
        Handled
    };
} // namespace ego::gui
