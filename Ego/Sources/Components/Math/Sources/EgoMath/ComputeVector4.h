#pragma once

#include "ComputeVector3.h"

namespace ego
{
    template <typename T>
    class ComputeVector4Base final
    {
    public:
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);

        using VectorType = Vector4Base<ValueType>;

    private:
        union
        {
            VectorType m_vector;

            struct
            {
                ComputeVector2Base<ValueType> m_vectorXY;
                ComputeVector2Base<ValueType> m_vectorZW;
            };

            ComputeVector3Base<ValueType> m_vectorXYZ;
        };

    public:
        constexpr ComputeVector4Base()
            : m_vector()
        {
        }
        constexpr ComputeVector4Base(ValueType _value)
            : m_vector(_value)
        {
        }
        constexpr ComputeVector4Base(ValueType _x, ValueType _y, ValueType _z, ValueType _w)
            : m_vector(_x, _y, _z, _w)
        {
        }
        constexpr ComputeVector4Base(const VectorType& _vector)
            : m_vector(_vector)
        {
        }
        constexpr ComputeVector4Base(const ComputeVector2Base<ValueType>& _vector)
            : m_vector(_vector.getX(), _vector.getY(), VectorType::DefaultValue, VectorType::DefaultValue)
        {
        }
        constexpr ComputeVector4Base(const ComputeVector3Base<ValueType>& _vector)
            : m_vector(_vector.getX(), _vector.getY(), _vector.getZ(), VectorType::DefaultValue)
        {
        }
        constexpr ComputeVector4Base(const ComputeVector2Base<ValueType>& _vector, ValueType _z, ValueType _w)
            : m_vector(_vector.getX(), _vector.getY(), _z, _w)
        {
        }
        constexpr ComputeVector4Base(const ComputeVector3Base<ValueType>& _vector, ValueType _w)
            : m_vector(_vector.getX(), _vector.getY(), _vector.getZ(), _w)
        {
        }
        constexpr ComputeVector4Base(const ComputeVector4Base& _vector)
            : m_vector(_vector.m_vector)
        {
        }

        ComputeVector4Base& operator=(const ComputeVector4Base& _vector);

        ComputeVector4Base& operator+=(const ComputeVector4Base& _vector);
        ComputeVector4Base& operator-=(const ComputeVector4Base& _vector);
        ComputeVector4Base& operator*=(T _value);
        ComputeVector4Base& operator/=(T _value);

        bool operator==(const ComputeVector4Base& _vector) const;
        bool operator!=(const ComputeVector4Base& _vector) const;

        FloatVector4 getFloatVector4() const;
        void getFloatVector4(FloatVector4& _out) const;

        ValueType operator[](uint32_t _index) const
        {
            return getElement(_index);
        }
        ValueType& operator[](uint32_t _index)
        {
            return getElement(_index);
        }

        ValueType getElement(uint32_t _index) const
        {
            return m_vector.getElement(_index);
        }
        ValueType& getElement(uint32_t _index)
        {
            return m_vector.getElement(_index);
        }

        ValueType getX() const
        {
            return m_vector.m_x;
        }
        ValueType getY() const
        {
            return m_vector.m_y;
        }
        ValueType getZ() const
        {
            return m_vector.m_z;
        }
        ValueType getW() const
        {
            return m_vector.m_w;
        }
        ValueType& getX()
        {
            return m_vector.m_x;
        }
        ValueType& getY()
        {
            return m_vector.m_y;
        }
        ValueType& getZ()
        {
            return m_vector.m_z;
        }
        ValueType& getW()
        {
            return m_vector.m_w;
        }
        void setX(ValueType _value)
        {
            m_vector.m_x = _value;
        }
        void setY(ValueType _value)
        {
            m_vector.m_y = _value;
        }
        void setZ(ValueType _value)
        {
            m_vector.m_z = _value;
        }
        void setW(ValueType _value)
        {
            m_vector.m_w = _value;
        }

        const ComputeVector2Base<ValueType>& getXY() const
        {
            return m_vectorXY;
        }
        const ComputeVector2Base<ValueType>& getZW() const
        {
            return m_vectorZW;
        }
        ComputeVector2Base<ValueType>& getXY()
        {
            return m_vectorXY;
        }
        ComputeVector2Base<ValueType>& getZW()
        {
            return m_vectorZW;
        }
        void setXY(const ComputeVector2Base<ValueType>& _vector)
        {
            m_vectorXY = _vector;
        }
        void setZW(const ComputeVector2Base<ValueType>& _vector)
        {
            m_vectorZW = _vector;
        }

        const ComputeVector3Base<ValueType>& getXYZ() const
        {
            return m_vectorXYZ;
        }
        ComputeVector3Base<ValueType>& getXYZ()
        {
            return m_vectorXYZ;
        }
        void setXYZ(const ComputeVector3Base<ValueType>& _vector)
        {
            m_vectorXYZ = _vector;
        }

        ValueType getLength() const;
        ValueType getLengthSqr() const;

        ComputeVector4Base& negate();

        ComputeVector4Base getUnit() const;
        ComputeVector4Base& normalize();

        ValueType dot(const ComputeVector4Base& _vector) const;

        bool isEqual(const ComputeVector4Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isLess(const ComputeVector4Base& _vector,
            ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this < vector
        bool isLessOrEqual(const ComputeVector4Base& _vector,
            ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this <= vector
        bool isGreater(const ComputeVector4Base& _vector,
            ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this > vector
        bool isGreaterOrEqual(const ComputeVector4Base& _vector,
            ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this >= vector

        bool isUnit() const;
        bool isZero() const;
    };

    template <typename T>
    ComputeVector4Base<T> operator+(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2);
    template <typename T>
    ComputeVector4Base<T> operator-(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2);
    template <typename T>
    ComputeVector4Base<T> operator-(const ComputeVector4Base<T>& _vector);
    template <typename T>
    ComputeVector4Base<T> operator*(const ComputeVector4Base<T>& _vector, T _value);
    template <typename T>
    ComputeVector4Base<T> operator*(T _value, const ComputeVector4Base<T>& _vector);
    template <typename T>
    ComputeVector4Base<T> operator/(const ComputeVector4Base<T>& _vector, T _value);
    template <typename T>
    ComputeVector4Base<T> operator/(T _value, const ComputeVector4Base<T>& _vector);

    template <typename T>
    ComputeVector4Base<T> ComputeVector4FromPoint(const ComputeVector3Base<T>& _vector);

    template <typename T>
    T DotComputeVector4(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2);

    using ComputeVector4 = ComputeVector4Base<ComputeValueType>;
    using FloatComputeVector4 = ComputeVector4Base<float>;
} // namespace ego

#include "ComputeVector4.hpp"