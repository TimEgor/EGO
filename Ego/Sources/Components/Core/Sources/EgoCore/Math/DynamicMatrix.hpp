#pragma once

#include "EgoCore/Assert/Assert.h"

namespace ego
{
#pragma region DynamicMatrixBase
    template <typename T>
    bool DynamicMatrixBase<T>::IsValidDimension(size_t _dimension)
    {
        return _dimension > 0 && _dimension <= std::numeric_limits<uint32_t>::max();
    }

    template <typename T>
    bool DynamicMatrixBase<T>::TryComputeElementCount(size_t _rowCount, size_t _columnCount, size_t& _elementCount)
    {
        _elementCount = 0;
        if (!IsValidDimension(_rowCount) || !IsValidDimension(_columnCount))
        {
            return false;
        }

        if (_columnCount > std::numeric_limits<size_t>::max() / _rowCount)
        {
            return false;
        }

        _elementCount = _rowCount * _columnCount;
        return true;
    }

    template <typename T>
    size_t DynamicMatrixBase<T>::getElementIndex(size_t _row, size_t _column) const
    {
        EGO_ASSERT(_row < m_rowCount);
        EGO_ASSERT(_column < m_columnCount);
        return _column * m_rowCount + _row;
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(size_t _dimensionRow, size_t _dimensionColumn)
    {
        size_t elementCount = 0;
        if (!TryComputeElementCount(_dimensionRow, _dimensionColumn, elementCount))
        {
            EGO_ASSERT(
                IsValidDimension(_dimensionRow) && IsValidDimension(_dimensionColumn) &&
                _dimensionColumn <= std::numeric_limits<size_t>::max() / _dimensionRow);

            return;
        }

        m_rowCount = _dimensionRow;
        m_columnCount = _dimensionColumn;
        m_values.resize(elementCount, DefaultValue);
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(ColumnContainer&& _columns)
    {
        if (_columns.empty())
        {
            EGO_ASSERT(!_columns.empty());

            return;
        }

        const size_t rowCount = _columns.front().getElementCount();
        const size_t columnCount = _columns.size();
        size_t elementCount = 0;
        if (!TryComputeElementCount(rowCount, columnCount, elementCount))
        {
            EGO_ASSERT(IsValidDimension(rowCount) && IsValidDimension(columnCount) && columnCount <= std::numeric_limits<size_t>::max() / rowCount);

            return;
        }

        for (const ColumnVectorType& column : _columns)
        {
            if (column.getElementCount() != rowCount)
            {
                EGO_ASSERT(column.getElementCount() == rowCount);

                return;
            }
        }

        ValueContainer values(elementCount, DefaultValue);
        for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex)
        {
            const typename ColumnVectorType::ConstValueView columnValues = _columns[columnIndex].getValues();
            std::copy(columnValues.begin(), columnValues.end(), values.begin() + columnIndex * rowCount);
        }

        m_rowCount = rowCount;
        m_columnCount = columnCount;
        m_values = std::move(values);
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(DynamicMatrixBase&& _matrix) noexcept
        : m_rowCount(_matrix.m_rowCount),
          m_columnCount(_matrix.m_columnCount),
          m_values(std::move(_matrix.m_values))
    {
        _matrix.m_rowCount = 0;
        _matrix.m_columnCount = 0;
        _matrix.m_values.clear();
    }

    template <typename T>
    DynamicMatrixBase<T>& DynamicMatrixBase<T>::operator=(DynamicMatrixBase&& _matrix) noexcept
    {
        if (this == &_matrix)
        {
            return *this;
        }

        m_rowCount = _matrix.m_rowCount;
        m_columnCount = _matrix.m_columnCount;
        m_values = std::move(_matrix.m_values);

        _matrix.m_rowCount = 0;
        _matrix.m_columnCount = 0;
        _matrix.m_values.clear();

        return *this;
    }

    template <typename T>
    typename DynamicMatrixBase<T>::ConstColumnView DynamicMatrixBase<T>::operator[](size_t _index) const
    {
        return getColumn(_index);
    }

    template <typename T>
    typename DynamicMatrixBase<T>::ColumnView DynamicMatrixBase<T>::operator[](size_t _index)
    {
        return getColumn(_index);
    }

    template <typename T>
    typename DynamicMatrixBase<T>::ConstColumnView DynamicMatrixBase<T>::getColumn(size_t _index) const
    {
        if (_index >= m_columnCount)
        {
            EGO_ASSERT(_index < m_columnCount);

            return ConstColumnView();
        }

        return ConstColumnView(m_values.data() + _index * m_rowCount, m_rowCount);
    }

    template <typename T>
    typename DynamicMatrixBase<T>::ColumnView DynamicMatrixBase<T>::getColumn(size_t _index)
    {
        if (_index >= m_columnCount)
        {
            EGO_ASSERT(_index < m_columnCount);

            return ColumnView();
        }

        return ColumnView(m_values.data() + _index * m_rowCount, m_rowCount);
    }

    template <typename T>
    typename DynamicMatrixBase<T>::RowVectorType DynamicMatrixBase<T>::getRow(size_t _index) const
    {
        if (_index >= m_rowCount)
        {
            EGO_ASSERT(_index < m_rowCount);

            return RowVectorType();
        }

        RowVectorType row(m_columnCount);
        for (size_t columnIndex = 0; columnIndex < m_columnCount; ++columnIndex)
        {
            row.setElement(columnIndex, getElement(_index, columnIndex));
        }

        return row;
    }

    template <typename T>
    void DynamicMatrixBase<T>::setRow(size_t _index, ConstValueView _values)
    {
        if (_index >= m_rowCount || _values.size() != m_columnCount)
        {
            EGO_ASSERT(_index < m_rowCount && _values.size() == m_columnCount);

            return;
        }

        const RowVectorType row(_values);
        for (size_t columnIndex = 0; columnIndex < m_columnCount; ++columnIndex)
        {
            setElement(_index, columnIndex, row.getElement(columnIndex));
        }
    }

    template <typename T>
    typename DynamicMatrixBase<T>::ValueType DynamicMatrixBase<T>::getElement(size_t _row, size_t _column) const
    {
        return m_values[getElementIndex(_row, _column)];
    }

    template <typename T>
    typename DynamicMatrixBase<T>::ValueType& DynamicMatrixBase<T>::getElement(size_t _row, size_t _column)
    {
        return m_values[getElementIndex(_row, _column)];
    }

    template <typename T>
    void DynamicMatrixBase<T>::setElement(size_t _row, size_t _column, ValueType _value)
    {
        m_values[getElementIndex(_row, _column)] = _value;
    }

    template <typename T>
    void DynamicMatrixBase<T>::reset()
    {
        std::fill(m_values.begin(), m_values.end(), DefaultValue);
    }

    template <typename T>
    uint32_t DynamicMatrixBase<T>::getRowCount() const
    {
        return static_cast<uint32_t>(m_rowCount);
    }

    template <typename T>
    uint32_t DynamicMatrixBase<T>::getColumnCount() const
    {
        return static_cast<uint32_t>(m_columnCount);
    }

#pragma endregion
} // namespace ego
