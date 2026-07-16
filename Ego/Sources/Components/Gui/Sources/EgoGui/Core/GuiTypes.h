#pragma once

#include <cstdint>

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego::gui
{
    class GuiFontAtlas;
    using GuiFontAtlasPointer = ego::SharedPointer<GuiFontAtlas>;

    using GuiTextureID = uint64_t;
    inline constexpr GuiTextureID InvalidGuiTextureID = 0;

    using GuiSize = FloatVector2;
    using GuiPosition = FloatVector2;
    using GuiColor = FloatVector4;

    inline constexpr GuiSize GuiSizeZero = FloatVector2Zero;
    inline constexpr GuiPosition GuiPositionZero = FloatVector2Zero;
    inline constexpr GuiColor GuiColorTransparent = FloatVector4(0.0f, 0.0f, 0.0f, 0.0f);
    inline constexpr GuiColor GuiColorWhite = FloatVector4(1.0f, 1.0f, 1.0f, 1.0f);

    enum class GuiVisibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    enum class GuiHorizontalAlignment
    {
        Left,
        Center,
        Right,
        Stretch
    };

    struct GuiRect final
    {
        GuiPosition m_position = GuiPositionZero;
        GuiSize m_size = GuiSizeZero;

        constexpr GuiRect() = default;

        constexpr GuiRect(const GuiPosition& _position, const GuiSize& _size)
            : m_position(_position),
              m_size(_size)
        {
        }

        constexpr GuiRect(float _x, float _y, float _width, float _height)
            : m_position(_x, _y),
              m_size(_width, _height)
        {
        }

        float getLeft() const;
        float getTop() const;
        float getRight() const;
        float getBottom() const;
        bool contains(const GuiPosition& _position) const;
    };

    struct GuiMargin final
    {
        float m_left = 0.0f;
        float m_top = 0.0f;
        float m_right = 0.0f;
        float m_bottom = 0.0f;

        constexpr GuiMargin() = default;

        constexpr GuiMargin(float _value)
            : m_left(_value),
              m_top(_value),
              m_right(_value),
              m_bottom(_value)
        {
        }

        constexpr GuiMargin(float _horizontal, float _vertical)
            : m_left(_horizontal),
              m_top(_vertical),
              m_right(_horizontal),
              m_bottom(_vertical)
        {
        }

        constexpr GuiMargin(float _left, float _top, float _right, float _bottom)
            : m_left(_left),
              m_top(_top),
              m_right(_right),
              m_bottom(_bottom)
        {
        }

        float getHorizontal() const;
        float getVertical() const;
    };

    struct GuiLayoutContext final
    {
        GuiFontAtlasPointer m_fontAtlas = nullptr;
    };
} // namespace ego::gui
