#pragma once

#include "Vector.h"

namespace ego
{
    template <typename T>
    struct Matrix2x2Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);

        static constexpr ValueType DefaultValue = 0;

        using RowVectorType = Vector2Base<ValueType>;

        union
        {
            ValueType m_values[2][2];

            struct
            {
                ValueType m_m11, m_m12;
                ValueType m_m21, m_m22;
            };

            RowVectorType m_rows[2];

            struct
            {
                RowVectorType m_row1;
                RowVectorType m_row2;
            };
        };

        constexpr Matrix2x2Base() = default;
        constexpr Matrix2x2Base(ValueType _m11, ValueType _m12, ValueType _m21, ValueType _m22)
            : m_m11(_m11),
              m_m12(_m12),
              m_m21(_m21),
              m_m22(_m22)
        {
        }

        constexpr Matrix2x2Base(const RowVectorType& _row1, const RowVectorType& _row2)
            : m_row1(_row1),
              m_row2(_row2)
        {
        }

        constexpr Matrix2x2Base(const Matrix2x2Base& _matrix)
            : m_row1(_matrix.m_row1),
              m_row2(_matrix.m_row2)
        {
        }

        Matrix2x2Base& operator=(const Matrix2x2Base& _matrix);
        const RowVectorType& operator[](size_t _index) const;
        RowVectorType& operator[](size_t _index);

        const RowVectorType& getRow(size_t _index) const;
        RowVectorType& getRow(size_t _index);

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);
    };

    template <typename T>
    struct Matrix3x3Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);

        static constexpr ValueType DefaultValue = 0;

        using RowVectorType = Vector3Base<ValueType>;

        union
        {
            ValueType m_values[3][3];

            struct
            {
                ValueType m_m11, m_m12, m_m13;
                ValueType m_m21, m_m22, m_m23;
                ValueType m_m31, m_m32, m_m33;
            };

            RowVectorType m_rows[3];

            struct
            {
                RowVectorType m_row1;
                RowVectorType m_row2;
                RowVectorType m_row3;
            };
        };

        constexpr Matrix3x3Base() = default;
        constexpr Matrix3x3Base(ValueType _m11, ValueType _m12, ValueType _m13, ValueType _m21, ValueType _m22, ValueType _m23, ValueType _m31, ValueType _m32, ValueType _m33)
            : m_m11(_m11),
              m_m12(_m12),
              m_m13(_m13),
              m_m21(_m21),
              m_m22(_m22),
              m_m23(_m23),
              m_m31(_m31),
              m_m32(_m32),
              m_m33(_m33)
        {
        }

        constexpr Matrix3x3Base(const RowVectorType& _row1, const RowVectorType& _row2, const RowVectorType& _row3)
            : m_row1(_row1),
              m_row2(_row2),
              m_row3(_row3)
        {
        }

        constexpr Matrix3x3Base(const Matrix2x2Base<ValueType>& _matrix)
            : m_row1(_matrix.m_row1),
              m_row2(_matrix.m_row2)
        {
        }

        constexpr Matrix3x3Base(const Matrix3x3Base& _matrix)
            : m_row1(_matrix.m_row1),
              m_row2(_matrix.m_row2),
              m_row3(_matrix.m_row3)
        {
        }

        Matrix3x3Base& operator=(const Matrix2x2Base<ValueType>& _matrix);
        Matrix3x3Base& operator=(const Matrix3x3Base& _matrix);
        const RowVectorType& operator[](size_t _index) const;
        RowVectorType& operator[](size_t _index);

        const RowVectorType& getRow(size_t _index) const;
        RowVectorType& getRow(size_t _index);

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);
    };

    template <typename T>
    struct Matrix4x4Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);

        static constexpr ValueType DefaultValue = 0;

        using RowVectorType = Vector4Base<ValueType>;

        union
        {
            ValueType m_values[4][4];

            struct
            {
                ValueType m_m11, m_m12, m_m13, m_m14;
                ValueType m_m21, m_m22, m_m23, m_m24;
                ValueType m_m31, m_m32, m_m33, m_m34;
                ValueType m_m41, m_m42, m_m43, m_m44;
            };

            RowVectorType m_rows[4];

            struct
            {
                RowVectorType m_row1;
                RowVectorType m_row2;
                RowVectorType m_row3;
                RowVectorType m_row4;
            };
        };

        constexpr Matrix4x4Base() = default;
        constexpr Matrix4x4Base(
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
            : m_m11(_m11),
              m_m12(_m12),
              m_m13(_m13),
              m_m14(_m14),
              m_m21(_m21),
              m_m22(_m22),
              m_m23(_m23),
              m_m24(_m24),
              m_m31(_m31),
              m_m32(_m32),
              m_m33(_m33),
              m_m34(_m34),
              m_m41(_m41),
              m_m42(_m42),
              m_m43(_m43),
              m_m44(_m44)
        {
        }

        constexpr Matrix4x4Base(const RowVectorType& _row1, const RowVectorType& _row2, const RowVectorType& _row3, const RowVectorType& _row4)
            : m_row1(_row1),
              m_row2(_row2),
              m_row3(_row3),
              m_row4(_row4)
        {
        }

        constexpr Matrix4x4Base(const Matrix2x2Base<ValueType>& _matrix)
            : m_row1(_matrix.m_row1),
              m_row2(_matrix.m_row2)
        {
        }

        constexpr Matrix4x4Base(const Matrix3x3Base<ValueType>& _matrix)
            : m_row1(_matrix.m_row1),
              m_row2(_matrix.m_row2),
              m_row3(_matrix.m_row3)
        {
        }

        constexpr Matrix4x4Base(const Matrix4x4Base& _matrix)
            : m_row1(_matrix.m_row1),
              m_row2(_matrix.m_row2),
              m_row3(_matrix.m_row3),
              m_row4(_matrix.m_row4)
        {
        }

        Matrix4x4Base& operator=(const Matrix2x2Base<ValueType>& _matrix);
        Matrix4x4Base& operator=(const Matrix3x3Base<ValueType>& _matrix);
        Matrix4x4Base& operator=(const Matrix4x4Base& _matrix);
        const RowVectorType& operator[](size_t _index) const;
        RowVectorType& operator[](size_t _index);

        const RowVectorType& getRow(size_t _index) const;
        RowVectorType& getRow(size_t _index);

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);
    };

    using FloatMatrix2x2 = Matrix2x2Base<float>;
    using FloatMatrix3x3 = Matrix3x3Base<float>;
    using FloatMatrix4x4 = Matrix4x4Base<float>;

    inline constexpr auto FloatMatrix2x2Zero = FloatMatrix2x2(0.0f, 0.0f, 0.0f, 0.0f);

    inline constexpr auto FloatMatrix2x2Identity = FloatMatrix2x2(1.0f, 0.0f, 0.0f, 1.0f);

    inline constexpr auto FloatMatrix3x3Zero = FloatMatrix3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    inline constexpr auto FloatMatrix3x3Identity = FloatMatrix3x3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    inline constexpr auto FloatMatrix4x4Zero = FloatMatrix4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    inline constexpr auto FloatMatrix4x4Identity = FloatMatrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    inline constexpr auto FloatMatrix4x4ZeroIdentity = FloatMatrix4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
} // namespace ego

#include "Matrix.hpp"
