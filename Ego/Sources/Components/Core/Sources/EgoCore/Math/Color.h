#pragma once

#include <type_traits>

#include "ComputeMath.h"
#include "Vector.h"

namespace ego
{
    using NormalizedColorValue = ComputeValue;
    using NormalizedColorVector3 = Vector3Base<NormalizedColorValue>;
    using NormalizedColorVector4 = Vector4Base<NormalizedColorValue>;

    struct NormalizedColorRGB final
    {
        NormalizedColorVector3 m_values = {};

        constexpr NormalizedColorRGB() = default;
        constexpr NormalizedColorRGB(const NormalizedColorVector3& _values)
            : m_values(_values)
        {
        }
        constexpr NormalizedColorRGB(NormalizedColorValue _r, NormalizedColorValue _g, NormalizedColorValue _b)
            : m_values(_r, _g, _b)
        {
        }

        constexpr NormalizedColorValue getR() const
        {
            return m_values.getX();
        }

        constexpr void setR(NormalizedColorValue _r)
        {
            m_values.setX(_r);
        }

        constexpr NormalizedColorValue getG() const
        {
            return m_values.getY();
        }

        constexpr void setG(NormalizedColorValue _g)
        {
            m_values.setY(_g);
        }

        constexpr NormalizedColorValue getB() const
        {
            return m_values.getZ();
        }

        constexpr void setB(NormalizedColorValue _b)
        {
            m_values.setZ(_b);
        }

        constexpr NormalizedColorRGB& operator=(const NormalizedColorVector3& _values)
        {
            m_values = _values;

            return *this;
        }
    };

    struct NormalizedColorRGBA final
    {
        NormalizedColorVector4 m_values = NormalizedColorVector4(NormalizedColorVector3(), NormalizedColorValue(1.0));

        constexpr NormalizedColorRGBA() = default;
        constexpr NormalizedColorRGBA(const NormalizedColorVector4& _values)
            : m_values(_values)
        {
        }
        constexpr NormalizedColorRGBA(NormalizedColorValue _r, NormalizedColorValue _g, NormalizedColorValue _b, NormalizedColorValue _a)
            : m_values(_r, _g, _b, _a)
        {
        }
        constexpr NormalizedColorRGBA(const NormalizedColorRGB& _color)
            : m_values(_color.m_values, NormalizedColorValue(1.0))
        {
        }

        constexpr NormalizedColorValue getR() const
        {
            return m_values.getX();
        }

        constexpr void setR(NormalizedColorValue _r)
        {
            m_values.setX(_r);
        }

        constexpr NormalizedColorValue getG() const
        {
            return m_values.getY();
        }

        constexpr void setG(NormalizedColorValue _g)
        {
            m_values.setY(_g);
        }

        constexpr NormalizedColorValue getB() const
        {
            return m_values.getZ();
        }

        constexpr void setB(NormalizedColorValue _b)
        {
            m_values.setZ(_b);
        }

        constexpr NormalizedColorValue getA() const
        {
            return m_values.getW();
        }

        constexpr void setA(NormalizedColorValue _a)
        {
            m_values.setW(_a);
        }

        constexpr NormalizedColorRGBA& operator=(const NormalizedColorVector4& _values)
        {
            m_values = _values;

            return *this;
        }

        constexpr NormalizedColorRGBA& operator=(const NormalizedColorRGB& _color)
        {
            m_values = NormalizedColorVector4(_color.m_values, NormalizedColorValue(1.0));

            return *this;
        }
    };

    static_assert(sizeof(NormalizedColorRGB) == sizeof(NormalizedColorVector3));
    static_assert(alignof(NormalizedColorRGB) == alignof(NormalizedColorVector3));
    static_assert(std::is_standard_layout_v<NormalizedColorRGB>);
    static_assert(std::is_trivially_copyable_v<NormalizedColorRGB>);
    static_assert(sizeof(NormalizedColorRGBA) == sizeof(NormalizedColorVector4));
    static_assert(alignof(NormalizedColorRGBA) == alignof(NormalizedColorVector4));
    static_assert(std::is_standard_layout_v<NormalizedColorRGBA>);
    static_assert(std::is_trivially_copyable_v<NormalizedColorRGBA>);

    inline constexpr NormalizedColorRGBA NormalizedColorTransparent =
        NormalizedColorRGBA(NormalizedColorValue(0.0), NormalizedColorValue(0.0), NormalizedColorValue(0.0), NormalizedColorValue(0.0));

    inline constexpr NormalizedColorRGB NormalizedColorWhite =
        NormalizedColorRGB(NormalizedColorValue(1.0), NormalizedColorValue(1.0), NormalizedColorValue(1.0));
    inline constexpr NormalizedColorRGB NormalizedColorGray =
        NormalizedColorRGB(NormalizedColorValue(0.5), NormalizedColorValue(0.5), NormalizedColorValue(0.5));
    inline constexpr NormalizedColorRGB NormalizedColorBlack =
        NormalizedColorRGB(NormalizedColorValue(0.0), NormalizedColorValue(0.0), NormalizedColorValue(0.0));

    inline constexpr NormalizedColorRGB NormalizedColorRed =
        NormalizedColorRGB(NormalizedColorValue(1.0), NormalizedColorValue(0.0), NormalizedColorValue(0.0));
    inline constexpr NormalizedColorRGB NormalizedColorGreen =
        NormalizedColorRGB(NormalizedColorValue(0.0), NormalizedColorValue(1.0), NormalizedColorValue(0.0));
    inline constexpr NormalizedColorRGB NormalizedColorBlue =
        NormalizedColorRGB(NormalizedColorValue(0.0), NormalizedColorValue(0.0), NormalizedColorValue(1.0));

    inline constexpr NormalizedColorRGB NormalizedColorSalmon =
        NormalizedColorRGB(NormalizedColorValue(0.9), NormalizedColorValue(0.6), NormalizedColorValue(0.5));
    inline constexpr NormalizedColorRGB NormalizedColorBrown =
        NormalizedColorRGB(NormalizedColorValue(0.65), NormalizedColorValue(0.15), NormalizedColorValue(0.15));
    inline constexpr NormalizedColorRGB NormalizedColorOrange =
        NormalizedColorRGB(NormalizedColorValue(1.0), NormalizedColorValue(0.5), NormalizedColorValue(0.0));
    inline constexpr NormalizedColorRGB NormalizedColorYellow =
        NormalizedColorRGB(NormalizedColorValue(1.0), NormalizedColorValue(1.0), NormalizedColorValue(0.0));
    inline constexpr NormalizedColorRGB NormalizedColorForestGreen =
        NormalizedColorRGB(NormalizedColorValue(0.15), NormalizedColorValue(0.55), NormalizedColorValue(0.15));
    inline constexpr NormalizedColorRGB NormalizedColorCyan =
        NormalizedColorRGB(NormalizedColorValue(0.0), NormalizedColorValue(1.0), NormalizedColorValue(1.0));
    inline constexpr NormalizedColorRGB NormalizedColorSteelBlue =
        NormalizedColorRGB(NormalizedColorValue(0.3), NormalizedColorValue(0.6), NormalizedColorValue(0.8));
    inline constexpr NormalizedColorRGB NormalizedColorViolet =
        NormalizedColorRGB(NormalizedColorValue(0.6), NormalizedColorValue(0.0), NormalizedColorValue(0.85));
    inline constexpr NormalizedColorRGB NormalizedColorMagenta =
        NormalizedColorRGB(NormalizedColorValue(1.0), NormalizedColorValue(0.0), NormalizedColorValue(1.0));
} // namespace ego
