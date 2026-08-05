#pragma once

#include <type_traits>

#include "Vector.h"

namespace ego
{
    template <typename T>
    struct QuaternionBase final
    {
        using VectorType = Vector4Base<T>;

        static_assert(std::is_arithmetic_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>);

        VectorType m_elements = {};

        constexpr QuaternionBase() = default;
        constexpr QuaternionBase(T _x, T _y, T _z, T _w)
            : m_elements(_x, _y, _z, _w)
        {
        }
        constexpr QuaternionBase(const VectorType& _elements)
            : m_elements(_elements)
        {
        }
        constexpr T getX() const
        {
            return m_elements.getX();
        }

        constexpr void setX(T _x)
        {
            m_elements.setX(_x);
        }

        constexpr T getY() const
        {
            return m_elements.getY();
        }

        constexpr void setY(T _y)
        {
            m_elements.setY(_y);
        }

        constexpr T getZ() const
        {
            return m_elements.getZ();
        }

        constexpr void setZ(T _z)
        {
            m_elements.setZ(_z);
        }

        constexpr T getW() const
        {
            return m_elements.getW();
        }

        constexpr void setW(T _w)
        {
            m_elements.setW(_w);
        }

        constexpr QuaternionBase& operator=(const QuaternionBase& _quaternion) = default;
    };

    using FloatQuaternion = QuaternionBase<float>;

    static_assert(sizeof(FloatQuaternion) == sizeof(FloatVector4));
    static_assert(alignof(FloatQuaternion) == alignof(FloatVector4));
    static_assert(std::is_standard_layout_v<FloatQuaternion>);
    static_assert(std::is_trivially_copyable_v<FloatQuaternion>);

    inline constexpr FloatQuaternion FloatQuaternionZero = FloatQuaternion(0.0f, 0.0f, 0.0f, 0.0f);
    inline constexpr FloatQuaternion FloatQuaternionIdentity = FloatQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
} // namespace ego
