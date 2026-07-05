#pragma once

#include <cstdint>

#include "GuiTypes.h"

namespace ego::gui
{
    enum class GuiInputEventType
    {
        Undefined,
        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel,
        KeyDown,
        KeyUp,
        TextInput
    };

    enum class GuiMouseButton
    {
        Left,
        Right,
        Middle
    };

    struct GuiInputEvent final
    {
        GuiInputEventType m_type = GuiInputEventType::Undefined;
        GuiPosition m_position = GuiPositionZero;
        bool m_hasPosition = false;
        GuiMouseButton m_mouseButton = GuiMouseButton::Left;
        float m_wheelDelta = 0.0f;
        uint32_t m_key = 0;
        uint32_t m_textCodepoint = 0;
    };

    struct GuiReply final
    {
        bool m_isHandled = false;

        static GuiReply Handled()
        {
            return GuiReply{true};
        }

        static GuiReply Unhandled()
        {
            return GuiReply{false};
        }
    };
} // namespace ego::gui
