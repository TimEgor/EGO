#pragma once

#include "ComputeVector4.h"
#include "Matrix.h"

namespace ego
{
    template <typename T, uint32_t Size>
    struct ComputeMatrixStorageTraits;

    template <typename T>
    struct ComputeMatrixStorageTraits<T, 2>
    {
        using StorageType = Matrix2x2Base<T>;
    };

    template <typename T>
    struct ComputeMatrixStorageTraits<T, 3>
    {
        using StorageType = Matrix3x3Base<T>;
    };

    template <typename T>
    struct ComputeMatrixStorageTraits<T, 4>
    {
        using StorageType = Matrix4x4Base<T>;
    };

    template <typename T, uint32_t Size>
    class ComputeMatrixBase final
    {
    public:
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);
        static_assert(Size >= 2 && Size <= 4);

        static constexpr uint32_t ElementCount = Size;

        using MatrixType = typename ComputeMatrixStorageTraits<ValueType, Size>::StorageType;
        using ColumnVectorType = ComputeVectorBase<ValueType, Size>;

        ComputeMatrixBase() = default;

        ComputeMatrixBase(ValueType _m11, ValueType _m12, ValueType _m21, ValueType _m22)
            requires(Size == 2);
        ComputeMatrixBase(
            ValueType _m11,
            ValueType _m12,
            ValueType _m13,
            ValueType _m21,
            ValueType _m22,
            ValueType _m23,
            ValueType _m31,
            ValueType _m32,
            ValueType _m33)
            requires(Size == 3);
        ComputeMatrixBase(
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
            requires(Size == 4);

        ComputeMatrixBase(const MatrixType& _matrix);

        ComputeMatrixBase(const ColumnVectorType& _column1, const ColumnVectorType& _column2)
            requires(Size == 2);
        ComputeMatrixBase(const ColumnVectorType& _column1, const ColumnVectorType& _column2, const ColumnVectorType& _column3)
            requires(Size == 3);
        ComputeMatrixBase(
            const ColumnVectorType& _column1,
            const ColumnVectorType& _column2,
            const ColumnVectorType& _column3,
            const ColumnVectorType& _column4)
            requires(Size == 4);

        ComputeMatrixBase(const ComputeMatrixBase<ValueType, 2>& _matrix)
            requires(Size == 3 || Size == 4);
        ComputeMatrixBase(const ComputeMatrixBase<ValueType, 3>& _matrix)
            requires(Size == 4);
        ComputeMatrixBase(const ComputeMatrixBase& _matrix) = default;

        ComputeMatrixBase& operator=(const ComputeMatrixBase& _matrix) = default;

        ComputeMatrixBase& operator+=(const ComputeMatrixBase& _matrix);
        ComputeMatrixBase& operator-=(const ComputeMatrixBase& _matrix);
        ComputeMatrixBase& operator*=(const ComputeMatrixBase& _matrix);
        ComputeMatrixBase& operator*=(ValueType _value);
        ComputeMatrixBase& operator/=(ValueType _value);

        bool operator==(const ComputeMatrixBase& _matrix) const;
        bool operator!=(const ComputeMatrixBase& _matrix) const;

        MatrixType getMatrix() const;
        void getMatrix(MatrixType& _out) const;

        FloatMatrix2x2 getFloatMatrix2x2() const
            requires(Size == 2);
        void getFloatMatrix2x2(FloatMatrix2x2& _out) const
            requires(Size == 2);

        FloatMatrix3x3 getFloatMatrix3x3() const
            requires(Size == 3);
        void getFloatMatrix3x3(FloatMatrix3x3& _out) const
            requires(Size == 3);

        FloatMatrix4x4 getFloatMatrix4x4() const
            requires(Size == 4);
        void getFloatMatrix4x4(FloatMatrix4x4& _out) const
            requires(Size == 4);

        ValueType getElement(uint32_t _row, uint32_t _column) const;
        void setElement(uint32_t _row, uint32_t _column, ValueType _value);

        const ColumnVectorType& getColumn(uint32_t _index) const;
        ColumnVectorType& getColumn(uint32_t _index);
        ColumnVectorType getRow(uint32_t _index) const;

        void setColumn(uint32_t _index, const ColumnVectorType& _column);
        void setRow(uint32_t _index, const ColumnVectorType& _row);

        ComputeMatrixBase& negate();

        ComputeMatrixBase& transpose();

        ValueType getDeterminant() const;
        ComputeMatrixBase& invert();

        ColumnVectorType transform(const ColumnVectorType& _vector) const;
        void transform(const ColumnVectorType& _vector, ColumnVectorType& _out) const;

        bool isEqual(const ComputeMatrixBase& _matrix, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;

    private:
        template <typename, uint32_t>
        friend class ComputeMatrixBase;

        ValueType getStoredElement(uint32_t _column, uint32_t _row) const;
        void setStoredElement(uint32_t _column, uint32_t _row, ValueType _value);

        ColumnVectorType m_columns[Size];
    };

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator+(const ComputeMatrixBase<T, Size>& _matrix1, const ComputeMatrixBase<T, Size>& _matrix2);
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator-(const ComputeMatrixBase<T, Size>& _matrix1, const ComputeMatrixBase<T, Size>& _matrix2);
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator-(const ComputeMatrixBase<T, Size>& _matrix);
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator*(const ComputeMatrixBase<T, Size>& _matrix1, const ComputeMatrixBase<T, Size>& _matrix2);
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator*(const ComputeMatrixBase<T, Size>& _matrix, T _value);
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator*(T _value, const ComputeMatrixBase<T, Size>& _matrix);
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator/(const ComputeMatrixBase<T, Size>& _matrix, T _value);

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator*(const ComputeMatrixBase<T, Size>& _matrix, const ComputeVectorBase<T, Size>& _vector);
} // namespace ego

#include "ComputeMatrix.hpp"
