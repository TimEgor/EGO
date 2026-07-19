#pragma once

#include <limits>

#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class FontAtlas;
    struct Theme;

    inline constexpr float UnboundedLayoutExtent = (std::numeric_limits<float>::max)();

    struct Margin final
    {
        float m_left = 0.0f;
        float m_top = 0.0f;
        float m_right = 0.0f;
        float m_bottom = 0.0f;

        constexpr Margin() = default;

        constexpr Margin(float _value)
            : m_left(_value),
              m_top(_value),
              m_right(_value),
              m_bottom(_value)
        {
        }

        constexpr Margin(float _horizontal, float _vertical)
            : m_left(_horizontal),
              m_top(_vertical),
              m_right(_horizontal),
              m_bottom(_vertical)
        {
        }

        constexpr Margin(float _left, float _top, float _right, float _bottom)
            : m_left(_left),
              m_top(_top),
              m_right(_right),
              m_bottom(_bottom)
        {
        }

        float getHorizontal() const;
        float getVertical() const;
    };

    struct LayoutConstraints final
    {
        Size m_maximumSize = SizeZero;

        constexpr LayoutConstraints() = default;

        constexpr explicit LayoutConstraints(const Size& _maximumSize)
            : m_maximumSize(_maximumSize)
        {
        }
    };

    struct LayoutContext final
    {
        ego::SharedPointer<FontAtlas> m_fontAtlas = nullptr;
        ego::SharedPointer<const Theme> m_theme = nullptr;

        const Theme& getTheme() const;
    };
} // namespace ego::gui
