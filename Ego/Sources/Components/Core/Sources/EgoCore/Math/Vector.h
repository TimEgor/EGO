#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ego
{
    template <typename T>
    struct Vector2Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

        ValueType m_x = DefaultValue;
        ValueType m_y = DefaultValue;

        constexpr Vector2Base() = default;
        constexpr Vector2Base(ValueType _value)
            : Vector2Base(_value, _value)
        {
        }
        constexpr Vector2Base(ValueType _x, ValueType _y)
            : m_x(_x),
              m_y(_y)
        {
        }
        template <typename SourceValueType>
        explicit constexpr Vector2Base(const Vector2Base<SourceValueType>& _vector)
            : m_x(static_cast<ValueType>(_vector.m_x)),
              m_y(static_cast<ValueType>(_vector.m_y))
        {
        }
        constexpr Vector2Base(const Vector2Base&) = default;

        Vector2Base& operator=(const Vector2Base&) = default;
        ValueType operator[](size_t _index) const;
        ValueType& operator[](size_t _index);

        constexpr ValueType getX() const;
        constexpr void setX(ValueType _x);
        constexpr ValueType getY() const;
        constexpr void setY(ValueType _y);

        ValueType getElement(size_t _index) const;
        ValueType& getElement(size_t _index);

        void setElement(size_t _index, ValueType _value);
    };

    template <typename T>
    struct Vector3Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

        ValueType m_x = DefaultValue;
        ValueType m_y = DefaultValue;
        ValueType m_z = DefaultValue;

        constexpr Vector3Base() = default;
        constexpr Vector3Base(ValueType _value)
            : Vector3Base(_value, _value, _value)
        {
        }
        constexpr Vector3Base(ValueType _x, ValueType _y, ValueType _z)
            : m_x(_x),
              m_y(_y),
              m_z(_z)
        {
        }
        template <typename SourceValueType>
        explicit constexpr Vector3Base(const Vector3Base<SourceValueType>& _vector)
            : m_x(static_cast<ValueType>(_vector.m_x)),
              m_y(static_cast<ValueType>(_vector.m_y)),
              m_z(static_cast<ValueType>(_vector.m_z))
        {
        }
        explicit constexpr Vector3Base(const Vector2Base<ValueType>& _vector)
            : m_x(_vector.m_x),
              m_y(_vector.m_y)
        {
        }
        constexpr Vector3Base(const Vector2Base<ValueType>& _vector, ValueType _z)
            : m_x(_vector.m_x),
              m_y(_vector.m_y),
              m_z(_z)
        {
        }
        constexpr Vector3Base(const Vector3Base&) = default;

        Vector3Base& operator=(const Vector2Base<ValueType>& _vector);
        Vector3Base& operator=(const Vector3Base&) = default;
        ValueType operator[](size_t _index) const;
        ValueType& operator[](size_t _index);

        constexpr ValueType getX() const;
        constexpr void setX(ValueType _x);
        constexpr ValueType getY() const;
        constexpr void setY(ValueType _y);
        constexpr ValueType getZ() const;
        constexpr void setZ(ValueType _z);

        ValueType getElement(size_t _index) const;
        ValueType& getElement(size_t _index);

        void setElement(size_t _index, ValueType _value);
    };

    template <typename T>
    struct Vector4Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

        ValueType m_x = DefaultValue;
        ValueType m_y = DefaultValue;
        ValueType m_z = DefaultValue;
        ValueType m_w = DefaultValue;

        constexpr Vector4Base() = default;
        constexpr Vector4Base(ValueType _value)
            : Vector4Base(_value, _value, _value, _value)
        {
        }
        constexpr Vector4Base(ValueType _x, ValueType _y, ValueType _z, ValueType _w)
            : m_x(_x),
              m_y(_y),
              m_z(_z),
              m_w(_w)
        {
        }
        template <typename SourceValueType>
        explicit constexpr Vector4Base(const Vector4Base<SourceValueType>& _vector)
            : m_x(static_cast<ValueType>(_vector.m_x)),
              m_y(static_cast<ValueType>(_vector.m_y)),
              m_z(static_cast<ValueType>(_vector.m_z)),
              m_w(static_cast<ValueType>(_vector.m_w))
        {
        }
        explicit constexpr Vector4Base(const Vector2Base<ValueType>& _vector)
            : m_x(_vector.m_x),
              m_y(_vector.m_y)
        {
        }
        constexpr Vector4Base(const Vector2Base<ValueType>& _vector1, const Vector2Base<ValueType>& _vector2)
            : m_x(_vector1.m_x),
              m_y(_vector1.m_y),
              m_z(_vector2.m_x),
              m_w(_vector2.m_y)
        {
        }
        constexpr Vector4Base(const Vector2Base<ValueType>& _vector, ValueType _z, ValueType _w)
            : m_x(_vector.m_x),
              m_y(_vector.m_y),
              m_z(_z),
              m_w(_w)
        {
        }
        explicit constexpr Vector4Base(const Vector3Base<ValueType>& _vector)
            : m_x(_vector.m_x),
              m_y(_vector.m_y),
              m_z(_vector.m_z)
        {
        }
        constexpr Vector4Base(const Vector3Base<ValueType>& _vector, ValueType _w)
            : m_x(_vector.m_x),
              m_y(_vector.m_y),
              m_z(_vector.m_z),
              m_w(_w)
        {
        }
        constexpr Vector4Base(const Vector4Base&) = default;

        Vector4Base& operator=(const Vector2Base<ValueType>& _vector);
        Vector4Base& operator=(const Vector3Base<ValueType>& _vector);
        Vector4Base& operator=(const Vector4Base&) = default;
        ValueType operator[](size_t _index) const;
        ValueType& operator[](size_t _index);

        constexpr ValueType getX() const;
        constexpr void setX(ValueType _x);
        constexpr ValueType getY() const;
        constexpr void setY(ValueType _y);
        constexpr ValueType getZ() const;
        constexpr void setZ(ValueType _z);
        constexpr ValueType getW() const;
        constexpr void setW(ValueType _w);

        ValueType getElement(size_t _index) const;
        ValueType& getElement(size_t _index);

        void setElement(size_t _index, ValueType _value);
    };

    using FloatVector2 = Vector2Base<float>;
    using FloatVector3 = Vector3Base<float>;
    using FloatVector4 = Vector4Base<float>;

    using UInt8Vector2 = Vector2Base<uint8_t>;
    using UInt8Vector3 = Vector3Base<uint8_t>;
    using UInt8Vector4 = Vector4Base<uint8_t>;

    using Int8Vector2 = Vector2Base<int8_t>;
    using Int8Vector3 = Vector3Base<int8_t>;
    using Int8Vector4 = Vector4Base<int8_t>;

    using UInt16Vector2 = Vector2Base<uint16_t>;
    using UInt16Vector3 = Vector3Base<uint16_t>;
    using UInt16Vector4 = Vector4Base<uint16_t>;

    using Int16Vector2 = Vector2Base<int16_t>;
    using Int16Vector3 = Vector3Base<int16_t>;
    using Int16Vector4 = Vector4Base<int16_t>;

    using UInt32Vector2 = Vector2Base<uint32_t>;
    using UInt32Vector3 = Vector3Base<uint32_t>;
    using UInt32Vector4 = Vector4Base<uint32_t>;

    using Int32Vector2 = Vector2Base<int32_t>;
    using Int32Vector3 = Vector3Base<int32_t>;
    using Int32Vector4 = Vector4Base<int32_t>;

    static_assert(std::is_standard_layout_v<FloatVector2> && std::is_trivially_copyable_v<FloatVector2>);
    static_assert(std::is_standard_layout_v<FloatVector3> && std::is_trivially_copyable_v<FloatVector3>);
    static_assert(std::is_standard_layout_v<FloatVector4> && std::is_trivially_copyable_v<FloatVector4>);
    static_assert(sizeof(FloatVector2) == sizeof(float) * 2 && offsetof(FloatVector2, m_y) == sizeof(float));
    static_assert(sizeof(FloatVector3) == sizeof(float) * 3 && offsetof(FloatVector3, m_z) == sizeof(float) * 2);
    static_assert(sizeof(FloatVector4) == sizeof(float) * 4 && offsetof(FloatVector4, m_w) == sizeof(float) * 3);

    inline constexpr FloatVector2 FloatVector2Zero = FloatVector2(0.0f);
    inline constexpr FloatVector3 FloatVector3Zero = FloatVector3(0.0f);
    inline constexpr FloatVector4 FloatVector4Zero = FloatVector4(0.0f);

    inline constexpr FloatVector2 FloatVector2One = FloatVector2(1.0f);
    inline constexpr FloatVector3 FloatVector3One = FloatVector3(1.0f);
    inline constexpr FloatVector4 FloatVector4One = FloatVector4(1.0f);

    inline constexpr FloatVector2 FloatVector2Epsilon = FloatVector2(std::numeric_limits<float>::epsilon());
    inline constexpr FloatVector3 FloatVector3Epsilon = FloatVector3(std::numeric_limits<float>::epsilon());
    inline constexpr FloatVector4 FloatVector4Epsilon = FloatVector4(std::numeric_limits<float>::epsilon());

    inline constexpr FloatVector2 FloatVector2NegativeOne = FloatVector2(-1.0f);
    inline constexpr FloatVector3 FloatVector3NegativeOne = FloatVector3(-1.0f);
    inline constexpr FloatVector4 FloatVector4NegativeOne = FloatVector4(-1.0f);

    inline constexpr UInt8Vector2 UInt8Vector2Zero = UInt8Vector2(0);
    inline constexpr UInt8Vector3 UInt8Vector3Zero = UInt8Vector3(0);
    inline constexpr UInt8Vector4 UInt8Vector4Zero = UInt8Vector4(0);

    inline constexpr Int8Vector2 Int8Vector2Zero = Int8Vector2(0);
    inline constexpr Int8Vector3 Int8Vector3Zero = Int8Vector3(0);
    inline constexpr Int8Vector4 Int8Vector4Zero = Int8Vector4(0);

    inline constexpr UInt16Vector2 UInt16Vector2Zero = UInt16Vector2(0);
    inline constexpr UInt16Vector3 UInt16Vector3Zero = UInt16Vector3(0);
    inline constexpr UInt16Vector4 UInt16Vector4Zero = UInt16Vector4(0);

    inline constexpr Int16Vector2 Int16Vector2Zero = Int16Vector2(0);
    inline constexpr Int16Vector3 Int16Vector3Zero = Int16Vector3(0);
    inline constexpr Int16Vector4 Int16Vector4Zero = Int16Vector4(0);

    inline constexpr UInt32Vector2 UInt32Vector2Zero = UInt32Vector2(0);
    inline constexpr UInt32Vector3 UInt32Vector3Zero = UInt32Vector3(0);
    inline constexpr UInt32Vector4 UInt32Vector4Zero = UInt32Vector4(0);

    inline constexpr Int32Vector2 Int32Vector2Zero = Int32Vector2(0);
    inline constexpr Int32Vector3 Int32Vector3Zero = Int32Vector3(0);
    inline constexpr Int32Vector4 Int32Vector4Zero = Int32Vector4(0);

    inline constexpr FloatVector3 FloatVector3UnitX = FloatVector3(1.0f, 0.0f, 0.0f);
    inline constexpr FloatVector3 FloatVector3UnitY = FloatVector3(0.0f, 1.0f, 0.0f);
    inline constexpr FloatVector3 FloatVector3UnitZ = FloatVector3(0.0f, 0.0f, 1.0f);
    inline constexpr FloatVector3 FloatVector3NegativeUnitX = FloatVector3(-1.0f, 0.0f, 0.0f);
    inline constexpr FloatVector3 FloatVector3NegativeUnitY = FloatVector3(0.0f, -1.0f, 0.0f);
    inline constexpr FloatVector3 FloatVector3NegativeUnitZ = FloatVector3(0.0f, 0.0f, -1.0f);
} // namespace ego

#include "Vector.hpp"
