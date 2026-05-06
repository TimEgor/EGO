#pragma once

#include "ComputeVector2.h"
#include "Matrix.h"

namespace ego
{
    template <typename T>
    class ComputeMatrix2x2Base final
    {
    public:
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);

        using MatrixType = Matrix2x2Base<ValueType>;

    private:
        union
        {
            MatrixType m_matrix; // store values in a column-major form

            ComputeVector2Base<ValueType> m_columns[2];

            struct
            {
                ComputeVector2Base<ValueType> m_column1;
                ComputeVector2Base<ValueType> m_column2;
            };
        };

    public:
        constexpr ComputeMatrix2x2Base()
            : m_matrix() {}

        constexpr ComputeMatrix2x2Base(
            ValueType _m11,
            ValueType _m12,
            ValueType _m21,
            ValueType _m22
        )
            : m_matrix(
                _m11,
                _m12,
                _m21,
                _m22
            )
        {}

        constexpr ComputeMatrix2x2Base(const MatrixType& _matrix)
            : m_matrix(_matrix) {}

        constexpr ComputeMatrix2x2Base(
            const ComputeVector2Base<ValueType>& _column1,
            const ComputeVector2Base<ValueType>& _column2
        )
            : m_column1(_column1),
              m_column2(_column2)
        {}

        constexpr ComputeMatrix2x2Base(const ComputeMatrix2x2Base& _matrix)
            : m_matrix(_matrix.m_matrix) {}

        ComputeMatrix2x2Base& operator=(const ComputeMatrix2x2Base& _matrix);

        ComputeMatrix2x2Base& operator+=(const ComputeMatrix2x2Base& _matrix);
        ComputeMatrix2x2Base& operator-=(const ComputeMatrix2x2Base& _matrix);
        ComputeMatrix2x2Base& operator*=(const ComputeMatrix2x2Base& _matrix);
        ComputeMatrix2x2Base& operator*=(ValueType _value);
        ComputeMatrix2x2Base& operator/=(ValueType _value);

        bool operator==(const ComputeMatrix2x2Base& _matrix) const;
        bool operator!=(const ComputeMatrix2x2Base& _matrix) const;

        FloatMatrix2x2 getFloatMatrix2x2() const;
        void getFloatMatrix2x2(FloatMatrix2x2& _out) const { _out = m_matrix; }

        ValueType getElement(uint32_t _row, uint32_t _column) const { return m_matrix.getElement(_column, _row); }
        ValueType& getElement(uint32_t _row, uint32_t _column) { return m_matrix.getElement(_column, _row); }

        void setElement(uint32_t _row, uint32_t _column, ValueType _value)
        {
            return m_matrix.setElement(_column, _row, _value);
        }

        const ComputeVector2Base<ValueType>& getColumn(uint32_t _index) const;
        ComputeVector2Base<ValueType>& getColumn(uint32_t _index);
        ComputeVector2Base<ValueType> getRow(uint32_t _index) const;

        void setColumn(uint32_t _index, const ComputeVector2Base<ValueType>& _column);
        void setRow(uint32_t _index, const ComputeVector2Base<ValueType>& _row);

        ComputeMatrix2x2Base& negate();

        ComputeMatrix2x2Base& transpose();

        ValueType getDeterminant() const;
        ComputeMatrix2x2Base& invert();

        ComputeVector2Base<ValueType> transform(const ComputeVector2Base<ValueType>& _vector) const;
        void transform(const ComputeVector2Base<ValueType>& _vector, ComputeVector2Base<ValueType>& _out) const;

        bool isEqual(const ComputeMatrix2x2Base& _matrix, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
    };

    template <typename T>
    ComputeMatrix2x2Base<T> operator+(const ComputeMatrix2x2Base<T>& _matrix1, const ComputeMatrix2x2Base<T>& _matrix2);
    template <typename T>
    ComputeMatrix2x2Base<T> operator-(const ComputeMatrix2x2Base<T>& _matrix1, const ComputeMatrix2x2Base<T>& _matrix2);
    template <typename T>
    ComputeMatrix2x2Base<T> operator-(const ComputeMatrix2x2Base<T>& _matrix);
    template <typename T>
    ComputeMatrix2x2Base<T> operator*(const ComputeMatrix2x2Base<T>& _matrix1, const ComputeMatrix2x2Base<T>& _matrix2);
    template <typename T>
    ComputeMatrix2x2Base<T> operator*(const ComputeMatrix2x2Base<T>& _matrix, T _value);
    template <typename T>
    ComputeMatrix2x2Base<T> operator*(T _value, const ComputeMatrix2x2Base<T>& _matrix);
    template <typename T>
    ComputeMatrix2x2Base<T> operator/(const ComputeMatrix2x2Base<T>& _matrix, T _value);
    template <typename T>
    ComputeMatrix2x2Base<T> operator/(T _value, const ComputeMatrix2x2Base<T>& _matrix);

    template <typename T>
    ComputeVector2Base<T> operator*(const ComputeMatrix2x2Base<T>& _matrix, const ComputeVector2Base<T>& _vector);

    template <typename T>
    ComputeMatrix2x2Base<T> InvertComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix);
    template <typename T>
    ComputeMatrix2x2Base<T> TransposeComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix);

    template <typename T>
    constexpr ComputeMatrix2x2Base<T> ComputeMatrix2x2ZeroBase()
    {
        return ComputeMatrix2x2Base<T>(
            T(0.0),
            T(0.0),
            T(0.0),
            T(0.0)
        );
    }

    template <typename T>
    constexpr ComputeMatrix2x2Base<T> ComputeMatrix2x2IdentityBase()
    {
        return ComputeMatrix2x2Base<T>(
            T(1.0),
            T(0.0),
            T(0.0),
            T(1.0)
        );
    }

    using ComputeMatrix2x2 = ComputeMatrix2x2Base<ComputeValueType>;
    using FloatComputeMatrix2x2 = ComputeMatrix2x2Base<float>;

    constexpr ComputeMatrix2x2 ComputeMatrix2x2Zero = ComputeMatrix2x2ZeroBase<ComputeValueType>();
    constexpr ComputeMatrix2x2 ComputeMatrix2x2Identity = ComputeMatrix2x2IdentityBase<ComputeValueType>();

    constexpr FloatComputeMatrix2x2 FloatComputeMatrix2x2Zero = ComputeMatrix2x2ZeroBase<float>();
    constexpr FloatComputeMatrix2x2 FloatComputeMatrix2x2Identity = ComputeMatrix2x2IdentityBase<float>();
}

#include "ComputeMatrix22.hpp"
