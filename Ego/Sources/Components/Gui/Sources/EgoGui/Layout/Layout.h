#pragma once

#include <cstddef>
#include <limits>

#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class FontAtlas;
    class Viewport;
    class Widget;
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
        class ViewportAccessor final : public NonInstanceable
        {
            friend class Viewport;

            static bool IsLayoutInvalidated(const Widget& _root);
            static void Layout(const LayoutContext& _context, Widget& _root, const Size& _size);
        };

        LayoutContext(ego::SharedPointer<FontAtlas> _fontAtlas, ego::SharedPointer<const Theme> _theme);

        Size measure(Widget& _widget, const LayoutConstraints& _constraints) const;
        void arrange(Widget& _widget, const Rect& _bounds) const;
        const ego::SharedPointer<FontAtlas>& getFontAtlas() const;
        const ego::SharedPointer<const Theme>& getThemePointer() const;
        const Theme& getTheme() const;

    private:
        void completeLayout(Widget& _root) const;
        bool completeLayoutLevel(Widget& _widget, const Widget& _root, size_t _level) const;

        ego::SharedPointer<FontAtlas> m_fontAtlas = nullptr;
        ego::SharedPointer<const Theme> m_theme = nullptr;
    };
} // namespace ego::gui
