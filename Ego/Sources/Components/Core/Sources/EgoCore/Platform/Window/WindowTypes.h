#pragma once

#include <cstdint>

namespace ego
{
    struct WindowSize final
    {
        uint16_t m_x = 0;
        uint16_t m_y = 0;

        constexpr WindowSize() = default;

        constexpr WindowSize(uint16_t _x, uint16_t _y)
            : m_x(_x),
              m_y(_y)
        {
        }
    };

    inline constexpr WindowSize DefaultWindowSize = WindowSize(0, 0);

    struct WindowPoint final
    {
        int32_t m_x = 0;
        int32_t m_y = 0;

        constexpr WindowPoint() = default;

        constexpr WindowPoint(int32_t _x, int32_t _y)
            : m_x(_x),
              m_y(_y)
        {
        }
    };

    inline constexpr WindowPoint DefaultWindowPoint = WindowPoint(0, 0);

    struct WindowArea final
    {
        using ValueType = uint16_t;

        ValueType m_top = 0;
        ValueType m_bottom = 0;
        ValueType m_right = 0;
        ValueType m_left = 0;

        constexpr WindowArea() = default;

        constexpr WindowArea(ValueType _top, ValueType _bottom, ValueType _right, ValueType _left)
            : m_top(_top),
              m_bottom(_bottom),
              m_right(_right),
              m_left(_left)
        {
        }
    };

    inline constexpr WindowArea DefaultWindowArea = WindowArea(0, 0, 0, 0);

    using WindowKeyboardKey = uint32_t;
    using WindowTextCodepoint = uint32_t;

    enum class WindowKeyboardInputAction
    {
        Pressed,
        Released
    };

    struct WindowKeyboardInputData final
    {
        WindowKeyboardKey m_key = 0;
        uint32_t m_scanCode = 0;
        bool m_isRepeat = false;
        bool m_isExtended = false;
        WindowKeyboardInputAction m_action = WindowKeyboardInputAction::Pressed;
    };

    struct WindowTextInputData final
    {
        WindowTextCodepoint m_codepoint = 0;
    };

    struct WindowDesc final
    {
        const char* m_title = "";
        WindowSize m_size = DefaultWindowSize;
        bool m_showOnInit = true;
    };
} // namespace ego
