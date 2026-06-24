#pragma once

#include "ComputeMatrix33.h"
#include "ComputeVector4.h"
#include "Matrix.h"

namespace ego
{
    template <typename T>
    class ComputeMatrix4x4Base final
    {
    public:
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);

        using MatrixType = Matrix4x4Base<ValueType>;

    private:
        union
        {
            MatrixType m_matrix; // store values in a transposed form (column-major)

            ComputeVector4Base<ValueType> m_columns[4];

            struct
            {
                ComputeVector4Base<ValueType> m_column1;
                ComputeVector4Base<ValueType> m_column2;
                ComputeVector4Base<ValueType> m_column3;
                ComputeVector4Base<ValueType> m_column4;
            };
        };

    public:
        constexpr ComputeMatrix4x4Base()
            : m_matrix()
        {
        }
        constexpr ComputeMatrix4x4Base(
            ValueType _m11,
            ValueType _m12,
            ValueType _m13,
            ValueType _m14,
            ValueType _m21,
            ValueType _m22,
            ValueType _m23,
            ValueType _m24,
            ValueType _m31,
            ValueType _m32,
            ValueType _m33,
            ValueType _m34,
            ValueType _m41,
            ValueType _m42,
            ValueType _m43,
            ValueType _m44)
            : m_matrix(_m11, _m12, _m13, _m14, _m21, _m22, _m23, _m24, _m31, _m32, _m33, _m34, _m41, _m42, _m43, _m44)
        {
        }

        constexpr ComputeMatrix4x4Base(const MatrixType& _matrix)
            : m_matrix(_matrix)
        {
        }

        constexpr ComputeMatrix4x4Base(
            const ComputeVector4Base<ValueType>& _column1,
            const ComputeVector4Base<ValueType>& _column2,
            const ComputeVector4Base<ValueType>& _column3,
            const ComputeVector4Base<ValueType>& _column4)
            : m_column1(_column1),
              m_column2(_column2),
              m_column3(_column3),
              m_column4(_column4)
        {
        }

        constexpr ComputeMatrix4x4Base(const ComputeMatrix2x2Base<ValueType>& _matrix)
            : m_column1(_matrix.getColumn(0)),
              m_column2(_matrix.getColumn(1)),
              m_column3(ValueType(0.0)),
              m_column4(ValueType(0.0))
        {
        }

        constexpr ComputeMatrix4x4Base(const ComputeMatrix3x3Base<ValueType>& _matrix)
            : m_column1(_matrix.getColumn(0)),
              m_column2(_matrix.getColumn(1)),
              m_column3(_matrix.getColumn(2)),
              m_column4(ValueType(0.0))
        {
        }

        constexpr ComputeMatrix4x4Base(const ComputeMatrix4x4Base& _matrix)
            : m_matrix(_matrix.m_matrix)
        {
        }

        ComputeMatrix4x4Base& operator=(const ComputeMatrix4x4Base& _matrix);

        ComputeMatrix4x4Base& operator+=(const ComputeMatrix4x4Base& _matrix);
        ComputeMatrix4x4Base& operator-=(const ComputeMatrix4x4Base& _matrix);
        ComputeMatrix4x4Base& operator*=(const ComputeMatrix4x4Base& _matrix);
        ComputeMatrix4x4Base& operator*=(ValueType _value);
        ComputeMatrix4x4Base& operator/=(ValueType _value);

        bool operator==(const ComputeMatrix4x4Base& _matrix) const;
        bool operator!=(const ComputeMatrix4x4Base& _matrix) const;

        FloatMatrix4x4 getFloatMatrix4x4() const;
        void getFloatMatrix4x4(FloatMatrix4x4& _out) const
        {
            _out = m_matrix;
        }

        ValueType getElement(uint32_t _row, uint32_t _column) const
        {
            return m_matrix.getElement(_column, _row);
        }
        ValueType& getElement(uint32_t _row, uint32_t _column)
        {
            return m_matrix.getElement(_column, _row);
        }
        void setElement(uint32_t _row, uint32_t _column, ValueType _value)
        {
            return m_matrix.setElement(_column, _row, _value);
        }

        const ComputeVector4Base<ValueType>& getColumn(uint32_t _index) const;
        ComputeVector4Base<ValueType> getRow(uint32_t _index) const;
        ComputeVector4Base<ValueType>& getColumn(uint32_t _index);

        void setColumn(uint32_t _index, const ComputeVector4Base<ValueType>& _column);
        void setRow(uint32_t _index, const ComputeVector4Base<ValueType>& _row);

        ComputeMatrix4x4Base& negate();

        ComputeMatrix4x4Base& transpose();

        ValueType getDeterminant() const;
        ComputeMatrix4x4Base& invert();

        ComputeVector4Base<ValueType> transform(const ComputeVector4Base<ValueType>& _vector) const;
        void transform(const ComputeVector4Base<ValueType>& _vector, ComputeVector4Base<ValueType>& _out) const;

        bool isEqual(const ComputeMatrix4x4Base& _matrix, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
    };

    template <typename T>
    ComputeMatrix4x4Base<T> operator+(const ComputeMatrix4x4Base<T>& _matrix1, const ComputeMatrix4x4Base<T>& _matrix2);
    template <typename T>
    ComputeMatrix4x4Base<T> operator-(const ComputeMatrix4x4Base<T>& _matrix1, const ComputeMatrix4x4Base<T>& _matrix2);
    template <typename T>
    ComputeMatrix4x4Base<T> operator-(const ComputeMatrix4x4Base<T>& _matrix);
    template <typename T>
    ComputeMatrix4x4Base<T> operator*(const ComputeMatrix4x4Base<T>& _matrix1, const ComputeMatrix4x4Base<T>& _matrix2);
    template <typename T>
    ComputeMatrix4x4Base<T> operator*(const ComputeMatrix4x4Base<T>& _matrix, T _value);
    template <typename T>
    ComputeMatrix4x4Base<T> operator*(T _value, const ComputeMatrix4x4Base<T>& _matrix);
    template <typename T>
    ComputeMatrix4x4Base<T> operator/(const ComputeMatrix4x4Base<T>& _matrix, T _value);
    template <typename T>
    ComputeMatrix4x4Base<T> operator/(T _value, const ComputeMatrix4x4Base<T>& _matrix);

    template <typename T>
    ComputeVector4Base<T> operator*(const ComputeMatrix4x4Base<T>& _matrix, const ComputeVector4Base<T>& _vector);

    template <typename T>
    ComputeMatrix4x4Base<T> CrossProductSkewSymmetricComputeMatrix4x4(const ComputeVector4Base<T>& _vector);
    template <typename T>
    ComputeMatrix4x4Base<T> TranslationComputeMatrix4x4(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(T _pitch, T _yaw, T _roll);
    template <typename T>
    ComputeMatrix4x4Base<T> ScaleComputeMatrix4x4(const ComputeVector3Base<T>& _scale);

    template <typename T>
    ComputeMatrix4x4Base<T> InvertComputeMatrix4x4(const ComputeMatrix4x4Base<T>& _matrix);

    template <typename T>
    constexpr ComputeMatrix4x4Base<T> ComputeMatrix4x4ZeroBase()
    {
        return ComputeMatrix4x4Base<T>(T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0));
    }

    template <typename T>
    constexpr ComputeMatrix4x4Base<T> ComputeMatrix4x4IdentityBase()
    {
        return ComputeMatrix4x4Base<T>(T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0));
    }

    template <typename T>
    constexpr ComputeMatrix4x4Base<T> ComputeMatrix4x4ZeroIdentityBase()
    {
        return ComputeMatrix4x4Base<T>(T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0));
    }

    using ComputeMatrix4x4 = ComputeMatrix4x4Base<ComputeValueType>;
    using FloatComputeMatrix4x4 = ComputeMatrix4x4Base<float>;

    inline constexpr ComputeMatrix4x4 ComputeMatrix4x4Zero = ComputeMatrix4x4ZeroBase<ComputeValueType>();
    inline constexpr ComputeMatrix4x4 ComputeMatrix4x4Identity = ComputeMatrix4x4IdentityBase<ComputeValueType>();
    inline constexpr ComputeMatrix4x4 ComputeMatrix4x4ZeroIdentity = ComputeMatrix4x4ZeroIdentityBase<ComputeValueType>();

    inline constexpr FloatComputeMatrix4x4 FloatComputeMatrix4x4Zero = ComputeMatrix4x4ZeroBase<float>();
    inline constexpr FloatComputeMatrix4x4 FloatComputeMatrix4x4Identity = ComputeMatrix4x4IdentityBase<float>();
    inline constexpr FloatComputeMatrix4x4 FloatComputeMatrix4x4ZeroIdentity = ComputeMatrix4x4ZeroIdentityBase<float>();
} // namespace ego

#include "ComputeMatrix44.hpp"