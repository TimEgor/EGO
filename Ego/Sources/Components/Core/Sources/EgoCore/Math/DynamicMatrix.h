#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "DynamicVector.h"

namespace ego
{
    template <typename T>
    class DynamicMatrixBase final
    {
    public:
        using ValueType = T;
        using ValueContainer = std::vector<ValueType>;
        using ColumnVectorType = DynamicVectorBase<ValueType>;
        using RowVectorType = DynamicVectorBase<ValueType>;
        using ColumnContainer = std::vector<ColumnVectorType>;
        using ColumnView = std::span<ValueType>;
        using ConstColumnView = std::span<const ValueType>;
        using ConstValueView = std::span<const ValueType>;

        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

    private:
        size_t m_rowCount = 0;
        size_t m_columnCount = 0;
        ValueContainer m_values;

        static bool IsValidDimension(size_t _dimension);
        static bool TryComputeElementCount(size_t _rowCount, size_t _columnCount, size_t& _elementCount);
        size_t getElementIndex(size_t _row, size_t _column) const;

    public:
        DynamicMatrixBase() = default;
        explicit DynamicMatrixBase(size_t _dimensionRow, size_t _dimensionColumn);
        explicit DynamicMatrixBase(ColumnContainer&& _columns);
        DynamicMatrixBase(const DynamicMatrixBase& _matrix) = default;
        DynamicMatrixBase(DynamicMatrixBase&& _matrix) noexcept;

        DynamicMatrixBase& operator=(const DynamicMatrixBase& _matrix) = default;
        DynamicMatrixBase& operator=(DynamicMatrixBase&& _matrix) noexcept;
        ConstColumnView operator[](size_t _index) const;
        ColumnView operator[](size_t _index);

        ConstColumnView getColumn(size_t _index) const;
        ColumnView getColumn(size_t _index);
        RowVectorType getRow(size_t _index) const;
        void setRow(size_t _index, ConstValueView _values);

        ValueType getElement(size_t _row, size_t _column) const;
        ValueType& getElement(size_t _row, size_t _column);

        void setElement(size_t _row, size_t _column, ValueType _value);

        void reset();

        uint32_t getRowCount() const;
        uint32_t getColumnCount() const;
    };

    using DynamicMatrix = DynamicMatrixBase<ComputeValue>;
    using FloatDynamicMatrix = DynamicMatrixBase<float>;
} // namespace ego

#include "DynamicMatrix.hpp"
