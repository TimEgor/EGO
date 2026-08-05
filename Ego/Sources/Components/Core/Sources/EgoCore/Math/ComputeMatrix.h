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
        static_assert(std::is_floating_point_v<ValueType>);
        static_assert(Size >= 2 && Size <= 4);

        static constexpr uint32_t RowCount = Size;
        static constexpr uint32_t ColumnCount = Size;
        static constexpr uint32_t ElementCount = RowCount * ColumnCount;

        using MatrixType = typename ComputeMatrixStorageTraits<ValueType, Size>::StorageType;
        using ColumnVectorType = ComputeVectorBase<ValueType, Size>;

        ComputeMatrixBase() = default;

        template <typename SourceValueType>
        ComputeMatrixBase(const Matrix2x2Base<SourceValueType>& _matrix) requires(Size == 2);
        template <typename SourceValueType>
        ComputeMatrixBase(const Matrix3x3Base<SourceValueType>& _matrix) requires(Size == 3);
        template <typename SourceValueType>
        ComputeMatrixBase(const Matrix4x4Base<SourceValueType>& _matrix) requires(Size == 4);

        ComputeMatrixBase(const ColumnVectorType& _column0, const ColumnVectorType& _column1) requires(Size == 2);
        ComputeMatrixBase(const ColumnVectorType& _column0, const ColumnVectorType& _column1, const ColumnVectorType& _column2) requires(Size == 3);
        ComputeMatrixBase(
            const ColumnVectorType& _column0,
            const ColumnVectorType& _column1,
            const ColumnVectorType& _column2,
            const ColumnVectorType& _column3) requires(Size == 4);

        ComputeMatrixBase(const ComputeMatrixBase<ValueType, 2>& _matrix) requires(Size == 3 || Size == 4);
        ComputeMatrixBase(const ComputeMatrixBase<ValueType, 3>& _matrix) requires(Size == 4);
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

        template <typename ResultValueType>
        typename ComputeMatrixStorageTraits<ResultValueType, Size>::StorageType getMatrix() const;
        template <typename ResultValueType>
        void getMatrix(typename ComputeMatrixStorageTraits<ResultValueType, Size>::StorageType& _out) const;

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
