#pragma once

#include "EgoCore/Assert/Assert.h"

namespace ego
{
#pragma region DynamicMatrixBase
    template <typename T>
    size_t DynamicMatrixBase<T>::getElementIndex(size_t _row, size_t _column) const
    {
        EGO_ASSERT(_row < m_rowCount);
        EGO_ASSERT(_column < m_columnCount);
        return _row * m_columnCount + _column;
    }

    template <typename T>
    void DynamicMatrixBase<T>::rebuildRows()
    {
        m_rows.clear();
        m_rows.reserve(m_rowCount);

        for (size_t rowIndex = 0; rowIndex < m_rowCount; ++rowIndex)
        {
            const size_t elementOffset = rowIndex * m_columnCount;
            typename RowType::ValueView rowView(m_values.data() + elementOffset, m_columnCount);
            m_rows.emplace_back(rowView);
        }
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(size_t _dimensionRow, size_t _dimensionColumn)
        : m_rowCount(_dimensionRow),
          m_columnCount(_dimensionColumn),
          m_values(_dimensionRow * _dimensionColumn)
    {
        EGO_ASSERT(_dimensionRow > 0);
        EGO_ASSERT(_dimensionColumn > 0);

        rebuildRows();
        reset();
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(RowContainer&& _values)
    {
        m_rowCount = _values.size();
        EGO_ASSERT(m_rowCount > 0);

        m_columnCount = _values[0].getElementCount();
        EGO_ASSERT(m_columnCount > 0);
        m_values.resize(m_rowCount * m_columnCount);

        for (size_t rowIndex = 0; rowIndex < m_rowCount; ++rowIndex)
        {
            EGO_ASSERT(m_columnCount == _values[rowIndex].getElementCount());

            for (size_t columnIndex = 0; columnIndex < m_columnCount; ++columnIndex)
            {
                m_values[rowIndex * m_columnCount + columnIndex] = _values[rowIndex].getElement(columnIndex);
            }
        }

        rebuildRows();
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(const DynamicMatrixBase& _matrix)
        : m_rowCount(_matrix.m_rowCount),
          m_columnCount(_matrix.m_columnCount),
          m_values(_matrix.m_values)
    {
        rebuildRows();
    }

    template <typename T>
    DynamicMatrixBase<T>::DynamicMatrixBase(DynamicMatrixBase&& _matrix)
        : m_rowCount(_matrix.m_rowCount),
          m_columnCount(_matrix.m_columnCount),
          m_values(std::move(_matrix.m_values))
    {
        rebuildRows();

        _matrix.m_rowCount = 0;
        _matrix.m_columnCount = 0;
        _matrix.m_rows.clear();
    }

    template <typename T>
    DynamicMatrixBase<T>& DynamicMatrixBase<T>::operator=(const DynamicMatrixBase& _matrix)
    {
        m_rowCount = _matrix.m_rowCount;
        m_columnCount = _matrix.m_columnCount;
        m_values = _matrix.m_values;
        rebuildRows();

        return *this;
    }

    template <typename T>
    DynamicMatrixBase<T>& DynamicMatrixBase<T>::operator=(DynamicMatrixBase&& _matrix)
    {
        m_rowCount = _matrix.m_rowCount;
        m_columnCount = _matrix.m_columnCount;
        m_values = std::move(_matrix.m_values);
        rebuildRows();

        _matrix.m_rowCount = 0;
        _matrix.m_columnCount = 0;
        _matrix.m_rows.clear();

        return *this;
    }

    template <typename T>
    const typename DynamicMatrixBase<T>::RowType& DynamicMatrixBase<T>::operator[](size_t _index) const
    {
        return getRow(_index);
    }

    template <typename T>
    typename DynamicMatrixBase<T>::RowType& DynamicMatrixBase<T>::operator[](size_t _index)
    {
        return getRow(_index);
    }

    template <typename T>
    const typename DynamicMatrixBase<T>::RowType& DynamicMatrixBase<T>::getRow(size_t _index) const
    {
        EGO_ASSERT(_index < m_rowCount);
        return m_rows[_index];
    }

    template <typename T>
    typename DynamicMatrixBase<T>::RowType& DynamicMatrixBase<T>::getRow(size_t _index)
    {
        EGO_ASSERT(_index < m_rowCount);
        return m_rows[_index];
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
        EGO_ASSERT(m_rowCount <= std::numeric_limits<uint32_t>::max());
        return static_cast<uint32_t>(m_rowCount);
    }

    template <typename T>
    uint32_t DynamicMatrixBase<T>::getColumnCount() const
    {
        EGO_ASSERT(m_columnCount <= std::numeric_limits<uint32_t>::max());
        return static_cast<uint32_t>(m_columnCount);
    }

#pragma endregion
} // namespace ego
