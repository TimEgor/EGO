#pragma once

#include "DynamicVector.h"

namespace ego
{
    template <typename T>
    class DynamicMatrixBase final
    {
    public:
        using ValueType = T;
        using RowType = DynamicVectorBase<ValueType>;
        using RowContainer = std::vector<RowType>;

        static_assert(std::is_arithmetic_v<ValueType>);

        static constexpr ValueType DefaultValue = 0;

    private:
        RowContainer m_values;

    public:
        DynamicMatrixBase() = default;
        DynamicMatrixBase(size_t _dimensionRow, size_t _dimensionColumn);
        DynamicMatrixBase(RowContainer&& _values);
        DynamicMatrixBase(const DynamicMatrixBase& _matrix);
        DynamicMatrixBase(DynamicMatrixBase&& _matrix);

        DynamicMatrixBase& operator=(const DynamicMatrixBase& _matrix);
        DynamicMatrixBase& operator=(DynamicMatrixBase&& _matrix);
        const RowType& operator[](size_t _index) const;
        RowType& operator[](size_t _index);

        const RowType& getRow(size_t _index) const;
        RowType& getRow(size_t _index);

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);

        void reset();

        uint32_t getRowCount() const;
        uint32_t getColumnCount() const;
    };

    using FloatDynamicMatrix = DynamicMatrixBase<float>;
} // namespace ego

#include "DynamicMatrix.hpp"
