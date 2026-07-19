#pragma once

#include "Vector.h"

namespace ego
{
    struct NormalizedColorRGB final
    {
        union
        {
            FloatVector3 m_values = FloatVector3Zero;

            struct
            {
                float m_r, m_g, m_b;
            };
        };

        constexpr NormalizedColorRGB() = default;
        constexpr NormalizedColorRGB(const FloatVector3& _values)
            : m_values(_values)
        {
        }
        constexpr NormalizedColorRGB(float _r, float _g, float _b)
            : m_values(_r, _g, _b)
        {
        }
        constexpr NormalizedColorRGB(const NormalizedColorRGB& _color)
            : m_values(_color.m_values)
        {
        }

        NormalizedColorRGB& operator=(const FloatVector3& _values)
        {
            m_values = _values;
            return *this;
        }

        NormalizedColorRGB& operator=(const NormalizedColorRGB& _color)
        {
            m_values = _color.m_values;
            return *this;
        }
    };

    struct NormalizedColorRGBA final
    {
        union
        {
            FloatVector4 m_values = {FloatVector3Zero, 1.0f};

            struct
            {
                float m_r, m_g, m_b, m_a;
            };
        };

        constexpr NormalizedColorRGBA() = default;
        constexpr NormalizedColorRGBA(const FloatVector4& _values)
            : m_values(_values)
        {
        }
        constexpr NormalizedColorRGBA(float _r, float _g, float _b, float _a)
            : m_values(_r, _g, _b, _a)
        {
        }
        constexpr NormalizedColorRGBA(const NormalizedColorRGB& _color)
            : m_values(_color.m_values, 1.0f)
        {
        }
        constexpr NormalizedColorRGBA(const NormalizedColorRGBA& _color)
            : m_values(_color.m_values)
        {
        }

        NormalizedColorRGBA& operator=(const FloatVector4& _values)
        {
            m_values = _values;
            return *this;
        }

        NormalizedColorRGBA& operator=(const NormalizedColorRGB& _color)
        {
            m_values = FloatVector4(_color.m_values, 1.0f);
            return *this;
        }

        NormalizedColorRGBA& operator=(const NormalizedColorRGBA& _color)
        {
            m_values = _color.m_values;
            return *this;
        }
    };

    static_assert(sizeof(NormalizedColorRGBA) == sizeof(FloatVector4));
    static_assert(alignof(NormalizedColorRGBA) == alignof(FloatVector4));

    inline constexpr NormalizedColorRGBA NormalizedColorTransparent = NormalizedColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);

    inline constexpr auto NormalizedColorWhite = NormalizedColorRGB(1.0f, 1.0f, 1.0f);
    inline constexpr auto NormalizedColorGray = NormalizedColorRGB(0.5f, 0.5f, 0.5f);
    inline constexpr auto NormalizedColorBlack = NormalizedColorRGB(0.0f, 0.0f, 0.0f);

    inline constexpr auto NormalizedColorRed = NormalizedColorRGB(1.0f, 0.0f, 0.0f);
    inline constexpr auto NormalizedColorGreen = NormalizedColorRGB(0.0f, 1.0f, 0.0f);
    inline constexpr auto NormalizedColorBlue = NormalizedColorRGB(0.0f, 0.0f, 1.0f);

    inline constexpr auto NormalizedColorSalmon = NormalizedColorRGB(0.9f, 0.6f, 0.5f);
    inline constexpr auto NormalizedColorBrown = NormalizedColorRGB(0.65f, 0.15f, 0.15f);
    inline constexpr auto NormalizedColorOrange = NormalizedColorRGB(1.0f, 0.5f, 0.0f);
    inline constexpr auto NormalizedColorYellow = NormalizedColorRGB(1.0f, 1.0f, 0.0f);
    inline constexpr auto NormalizedColorForestGreen = NormalizedColorRGB(0.15f, 0.55f, 0.15f);
    inline constexpr auto NormalizedColorCyan = NormalizedColorRGB(0.0f, 1.0f, 1.0f);
    inline constexpr auto NormalizedColorSteelBlue = NormalizedColorRGB(0.3f, 0.6f, 0.8f);
    inline constexpr auto NormalizedColorViolet = NormalizedColorRGB(0.6f, 0.0f, 0.85f);
    inline constexpr auto NormalizedColorMagenta = NormalizedColorRGB(1.0f, 0.0f, 1.0f);
} // namespace ego
