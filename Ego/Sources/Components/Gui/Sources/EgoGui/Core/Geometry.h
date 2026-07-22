#pragma once

#include "EgoCore/Math/Vector.h"

namespace ego::gui
{
    using Size = FloatVector2;
    using Position = FloatVector2;

    inline constexpr Size SizeZero = FloatVector2Zero;
    inline constexpr Position PositionZero = FloatVector2Zero;

    bool AreEqual(const FloatVector2& _first, const FloatVector2& _second);

    struct Rect final
    {
        Position m_position = PositionZero;
        Size m_size = SizeZero;

        constexpr Rect() = default;

        constexpr Rect(const Position& _position, const Size& _size)
            : m_position(_position),
              m_size(_size)
        {
        }

        constexpr Rect(float _x, float _y, float _width, float _height)
            : m_position(_x, _y),
              m_size(_width, _height)
        {
        }

        float getLeft() const;
        float getTop() const;
        float getRight() const;
        float getBottom() const;
        bool contains(const Position& _position) const;

        bool operator==(const Rect& _rect) const;
    };
} // namespace ego::gui
