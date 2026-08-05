#pragma once

#include <cstddef>
#include <type_traits>

#include "Vector.h"

namespace ego
{
    template <typename T>
    struct Matrix2x2Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

        using ColumnVectorType = Vector2Base<ValueType>;
        using RowVectorType = Vector2Base<ValueType>;

        ColumnVectorType m_columns[2] = {};

        constexpr Matrix2x2Base() = default;

        constexpr Matrix2x2Base(const ColumnVectorType& _column0, const ColumnVectorType& _column1)
            : m_columns{_column0, _column1}
        {
        }
        template <typename SourceValueType>
        explicit constexpr Matrix2x2Base(const Matrix2x2Base<SourceValueType>& _matrix)
            : m_columns{ColumnVectorType(_matrix.m_columns[0]), ColumnVectorType(_matrix.m_columns[1])}
        {
        }

        constexpr Matrix2x2Base(const Matrix2x2Base&) = default;

        Matrix2x2Base& operator=(const Matrix2x2Base&) = default;
        const ColumnVectorType& operator[](size_t _index) const;
        ColumnVectorType& operator[](size_t _index);

        const ColumnVectorType& getColumn(size_t _index) const;
        ColumnVectorType& getColumn(size_t _index);
        RowVectorType getRow(size_t _index) const;

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);
    };

    template <typename T>
    struct Matrix3x3Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

        using ColumnVectorType = Vector3Base<ValueType>;
        using RowVectorType = Vector3Base<ValueType>;

        ColumnVectorType m_columns[3] = {};

        constexpr Matrix3x3Base() = default;

        constexpr Matrix3x3Base(const ColumnVectorType& _column0, const ColumnVectorType& _column1, const ColumnVectorType& _column2)
            : m_columns{_column0, _column1, _column2}
        {
        }
        template <typename SourceValueType>
        explicit constexpr Matrix3x3Base(const Matrix3x3Base<SourceValueType>& _matrix)
            : m_columns{ColumnVectorType(_matrix.m_columns[0]), ColumnVectorType(_matrix.m_columns[1]), ColumnVectorType(_matrix.m_columns[2])}
        {
        }

        explicit constexpr Matrix3x3Base(const Matrix2x2Base<ValueType>& _matrix)
            : m_columns{ColumnVectorType(_matrix.m_columns[0]), ColumnVectorType(_matrix.m_columns[1]), ColumnVectorType(DefaultValue)}
        {
        }

        constexpr Matrix3x3Base(const Matrix3x3Base&) = default;

        Matrix3x3Base& operator=(const Matrix2x2Base<ValueType>& _matrix);
        Matrix3x3Base& operator=(const Matrix3x3Base&) = default;
        const ColumnVectorType& operator[](size_t _index) const;
        ColumnVectorType& operator[](size_t _index);

        const ColumnVectorType& getColumn(size_t _index) const;
        ColumnVectorType& getColumn(size_t _index);
        RowVectorType getRow(size_t _index) const;

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);
    };

    template <typename T>
    struct Matrix4x4Base final
    {
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

        using ColumnVectorType = Vector4Base<ValueType>;
        using RowVectorType = Vector4Base<ValueType>;

        ColumnVectorType m_columns[4] = {};

        constexpr Matrix4x4Base() = default;

        constexpr Matrix4x4Base(
            const ColumnVectorType& _column0,
            const ColumnVectorType& _column1,
            const ColumnVectorType& _column2,
            const ColumnVectorType& _column3)
            : m_columns{_column0, _column1, _column2, _column3}
        {
        }
        template <typename SourceValueType>
        explicit constexpr Matrix4x4Base(const Matrix4x4Base<SourceValueType>& _matrix)
            : m_columns{ColumnVectorType(_matrix.m_columns[0]),
                  ColumnVectorType(_matrix.m_columns[1]),
                  ColumnVectorType(_matrix.m_columns[2]),
                  ColumnVectorType(_matrix.m_columns[3])}
        {
        }

        explicit constexpr Matrix4x4Base(const Matrix2x2Base<ValueType>& _matrix)
            : m_columns{ColumnVectorType(_matrix.m_columns[0]),
                  ColumnVectorType(_matrix.m_columns[1]),
                  ColumnVectorType(DefaultValue),
                  ColumnVectorType(DefaultValue)}
        {
        }

        explicit constexpr Matrix4x4Base(const Matrix3x3Base<ValueType>& _matrix)
            : m_columns{ColumnVectorType(_matrix.m_columns[0]),
                  ColumnVectorType(_matrix.m_columns[1]),
                  ColumnVectorType(_matrix.m_columns[2]),
                  ColumnVectorType(DefaultValue)}
        {
        }

        constexpr Matrix4x4Base(const Matrix4x4Base&) = default;

        Matrix4x4Base& operator=(const Matrix2x2Base<ValueType>& _matrix);
        Matrix4x4Base& operator=(const Matrix3x3Base<ValueType>& _matrix);
        Matrix4x4Base& operator=(const Matrix4x4Base&) = default;
        const ColumnVectorType& operator[](size_t _index) const;
        ColumnVectorType& operator[](size_t _index);

        const ColumnVectorType& getColumn(size_t _index) const;
        ColumnVectorType& getColumn(size_t _index);
        RowVectorType getRow(size_t _index) const;

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);
    };

    using FloatMatrix2x2 = Matrix2x2Base<float>;
    using FloatMatrix3x3 = Matrix3x3Base<float>;
    using FloatMatrix4x4 = Matrix4x4Base<float>;

    static_assert(std::is_standard_layout_v<FloatMatrix2x2> && std::is_trivially_copyable_v<FloatMatrix2x2>);
    static_assert(std::is_standard_layout_v<FloatMatrix3x3> && std::is_trivially_copyable_v<FloatMatrix3x3>);
    static_assert(std::is_standard_layout_v<FloatMatrix4x4> && std::is_trivially_copyable_v<FloatMatrix4x4>);
    static_assert(sizeof(FloatMatrix2x2) == sizeof(float) * 4 && offsetof(FloatMatrix2x2, m_columns) == 0);
    static_assert(sizeof(FloatMatrix3x3) == sizeof(float) * 9 && offsetof(FloatMatrix3x3, m_columns) == 0);
    static_assert(sizeof(FloatMatrix4x4) == sizeof(float) * 16 && offsetof(FloatMatrix4x4, m_columns) == 0);

    inline constexpr FloatMatrix2x2 FloatMatrix2x2Zero = FloatMatrix2x2();

    inline constexpr FloatMatrix2x2 FloatMatrix2x2Identity = FloatMatrix2x2(FloatVector2(1.0f, 0.0f), FloatVector2(0.0f, 1.0f));

    inline constexpr FloatMatrix3x3 FloatMatrix3x3Zero = FloatMatrix3x3();

    inline constexpr FloatMatrix3x3 FloatMatrix3x3Identity =
        FloatMatrix3x3(FloatVector3(1.0f, 0.0f, 0.0f), FloatVector3(0.0f, 1.0f, 0.0f), FloatVector3(0.0f, 0.0f, 1.0f));

    inline constexpr FloatMatrix4x4 FloatMatrix4x4Zero = FloatMatrix4x4();

    inline constexpr FloatMatrix4x4 FloatMatrix4x4Identity = FloatMatrix4x4(
        FloatVector4(1.0f, 0.0f, 0.0f, 0.0f),
        FloatVector4(0.0f, 1.0f, 0.0f, 0.0f),
        FloatVector4(0.0f, 0.0f, 1.0f, 0.0f),
        FloatVector4(0.0f, 0.0f, 0.0f, 1.0f));

    inline constexpr FloatMatrix4x4 FloatMatrix4x4ZeroIdentity =
        FloatMatrix4x4(FloatVector4Zero, FloatVector4Zero, FloatVector4Zero, FloatVector4(0.0f, 0.0f, 0.0f, 1.0f));
} // namespace ego

#include "Matrix.hpp"
