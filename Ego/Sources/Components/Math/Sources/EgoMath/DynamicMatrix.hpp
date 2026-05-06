#pragma once

#include "EgoCore/Assert/AssertCore.h"

namespace ego
{
#pragma region DynamicMatrixBase
	template <typename T>
	DynamicMatrixBase<T>::DynamicMatrixBase(size_t _dimensionRow, size_t _dimensionColumn)
	{
		EGO_ASSERT(_dimensionRow > 0);
		EGO_ASSERT(_dimensionColumn > 0);

		m_values = std::vector<RowType>(_dimensionRow);

		for (size_t rowIndex = 0; rowIndex < _dimensionRow; ++rowIndex)
		{
			m_values[rowIndex] = RowType(_dimensionColumn);
		}

		reset();
	}

	template <typename T>
	DynamicMatrixBase<T>::DynamicMatrixBase(RowContainer&& _values)
		: m_values(std::move(_values))
	{
		const size_t rowCount = m_values.size();
		EGO_ASSERT(rowCount > 0);

		const size_t columnCheckSize = m_values[0].getElementCount();

		for (size_t rowIndex = 1; rowIndex < rowCount; ++rowIndex)
		{
			EGO_ASSERT(columnCheckSize == m_values[rowIndex].getElementCount());
		}
	}


	template <typename T>
	DynamicMatrixBase<T>::DynamicMatrixBase(const DynamicMatrixBase& _matrix)
		: m_values(_matrix.m_values)
	{
	}

	template <typename T>
	DynamicMatrixBase<T>::DynamicMatrixBase(DynamicMatrixBase&& _matrix)
		: m_values(std::move(_matrix.m_values))
	{
	}

	template <typename T>
	DynamicMatrixBase<T>& DynamicMatrixBase<T>::operator=(const DynamicMatrixBase& _matrix)
	{
		m_values = _matrix.m_values;
		return *this;
	}

	template <typename T>
	DynamicMatrixBase<T>& DynamicMatrixBase<T>::operator=(DynamicMatrixBase&& _matrix)
	{
		m_values = std::move(_matrix.m_values);
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
		//EGO_ASSERT(_index < m_values.size());
		return m_values[_index];
	}

	template <typename T>
	typename DynamicMatrixBase<T>::RowType& DynamicMatrixBase<T>::getRow(size_t _index)
	{
		//EGO_ASSERT(_index < m_values.size());
		return m_values[_index];
	}

	template <typename T>
	const typename DynamicMatrixBase<T>::ValueType& DynamicMatrixBase<T>::getElement(size_t _row, size_t _column) const
	{
		//EGO_ASSERT(_row < m_values.size());
		//EGO_ASSERT(_column < m_values[_row].size());
		return m_values[_row][_column];
	}

	template <typename T>
	typename DynamicMatrixBase<T>::ValueType& DynamicMatrixBase<T>::getElement(size_t _row, size_t _column)
	{
		//EGO_ASSERT(_row < m_values.size());
		//EGO_ASSERT(_column < m_values[_row].size());
		return m_values[_row][_column];
	}

	template <typename T>
	void DynamicMatrixBase<T>::setElement(size_t _row, size_t _column, ValueType _value)
	{
		//EGO_ASSERT(_row < m_values.size());
		//EGO_ASSERT(_column < m_values[_row].size());
		m_values[_row][_column] = _value;
	}

	template <typename T>
	void DynamicMatrixBase<T>::reset()
	{
		const size_t rows = m_values.size();
		for (size_t rowIndex = 0; rowIndex < rows; ++rowIndex)
		{
			m_values[rowIndex].reset();
		}
	}

	template <typename T>
	uint32_t DynamicMatrixBase<T>::getRowCount() const
	{
		return m_values.size();
	}

	template <typename T>
	uint32_t DynamicMatrixBase<T>::getColumnCount() const
	{
		return m_values.empty() ? 0 : m_values[0].getElementCount();
	}

#pragma endregion
}
