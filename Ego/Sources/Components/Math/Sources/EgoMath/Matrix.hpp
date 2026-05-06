#pragma once

#include "EgoCore/Assert/AssertCore.h"

namespace ego
{
#pragma region Matrix2x2Base
	template <typename T>
	Matrix2x2Base<T>& Matrix2x2Base<T>::operator=(const Matrix2x2Base& _matrix)
	{
		m_row1 = _matrix.m_row1;
		m_row2 = _matrix.m_row2;

		return *this;
	}

	template <typename T>
	const typename Matrix2x2Base<T>::RowVectorType& Matrix2x2Base<T>::operator[](size_t _index) const
	{
		return getRow(_index);
	}

	template <typename T>
	typename Matrix2x2Base<T>::RowVectorType& Matrix2x2Base<T>::operator[](size_t _index)
	{
		return getRow(_index);
	}

	template <typename T>
	const typename Matrix2x2Base<T>::RowVectorType& Matrix2x2Base<T>::getRow(size_t _index) const
	{
		EGO_ASSERT(_index < 2);
		return m_rows[_index];
	}

	template <typename T>
	typename Matrix2x2Base<T>::RowVectorType& Matrix2x2Base<T>::getRow(size_t _index)
	{
		EGO_ASSERT(_index < 2);
		return m_rows[_index];
	}

	template <typename T>
	typename Matrix2x2Base<T>::ValueType Matrix2x2Base<T>::getElement(size_t _row, size_t _column) const
	{
		EGO_ASSERT(_row < 2 && _column < 2);
		return m_values[_row][_column];
	}

	template <typename T>
	typename Matrix2x2Base<T>::ValueType& Matrix2x2Base<T>::getElement(size_t _row, size_t _column)
	{
		EGO_ASSERT(_row < 2 && _column < 2);
		return m_values[_row][_column];
	}

	template <typename T>
	void Matrix2x2Base<T>::setElement(size_t _row, size_t _column, ValueType _value)
	{
		getElement(_row, _column) = _value;
	}

#pragma endregion

#pragma region Matrix3x3Base
	template <typename T>
	Matrix3x3Base<T>& Matrix3x3Base<T>::operator=(const Matrix2x2Base<ValueType>& _matrix)
	{
		m_row1 = _matrix.m_row1;
		m_row2 = _matrix.m_row2;
		m_row3 = RowVectorType(DefaultValue);

		return *this;
	}

	template <typename T>
	Matrix3x3Base<T>& Matrix3x3Base<T>::operator=(const Matrix3x3Base& _matrix)
	{
		m_row1 = _matrix.m_row1;
		m_row2 = _matrix.m_row2;
		m_row3 = _matrix.m_row3;

		return *this;
	}

	template <typename T>
	const typename Matrix3x3Base<T>::RowVectorType& Matrix3x3Base<T>::operator[](size_t _index) const
	{
		return getRow(_index);
	}

	template <typename T>
	typename Matrix3x3Base<T>::RowVectorType& Matrix3x3Base<T>::operator[](size_t _index)
	{
		return getRow(_index);
	}

	template <typename T>
	const typename Matrix3x3Base<T>::RowVectorType& Matrix3x3Base<T>::getRow(size_t _index) const
	{
		EGO_ASSERT(_index < 3);
		return m_rows[_index];
	}

	template <typename T>
	typename Matrix3x3Base<T>::RowVectorType& Matrix3x3Base<T>::getRow(size_t _index)
	{
		EGO_ASSERT(_index < 3);
		return m_rows[_index];
	}

	template <typename T>
	typename Matrix3x3Base<T>::ValueType Matrix3x3Base<T>::getElement(size_t _row, size_t _column) const
	{
		EGO_ASSERT(_row < 3 && _column < 3);
		return m_values[_row][_column];
	}

	template <typename T>
	typename Matrix3x3Base<T>::ValueType& Matrix3x3Base<T>::getElement(size_t _row, size_t _column)
	{
		EGO_ASSERT(_row < 3 && _column < 3);
		return m_values[_row][_column];
	}

	template <typename T>
	void Matrix3x3Base<T>::setElement(size_t _row, size_t _column, ValueType _value)
	{
		getElement(_row, _column) = _value;
	}
#pragma endregion

#pragma region Matrix4x4Base
	template <typename T>
	Matrix4x4Base<T>& Matrix4x4Base<T>::operator=(const Matrix2x2Base<ValueType>& _matrix)
	{
		m_row1 = _matrix.m_row1;
		m_row2 = _matrix.m_row2;
		m_row3 = RowVectorType(DefaultValue);
		m_row4 = RowVectorType(DefaultValue);

		return *this;
	}

	template <typename T>
	Matrix4x4Base<T>& Matrix4x4Base<T>::operator=(const Matrix3x3Base<ValueType>& _matrix)
	{
		m_row1 = _matrix.m_row1;
		m_row2 = _matrix.m_row2;
		m_row3 = _matrix.m_row3;
		m_row4 = RowVectorType(DefaultValue);

		return *this;
	}

	template <typename T>
	Matrix4x4Base<T>& Matrix4x4Base<T>::operator=(const Matrix4x4Base& _matrix)
	{
		m_row1 = _matrix.m_row1;
		m_row2 = _matrix.m_row2;
		m_row3 = _matrix.m_row3;
		m_row4 = _matrix.m_row4;

		return *this;
	}

	template <typename T>
	const typename Matrix4x4Base<T>::RowVectorType& Matrix4x4Base<T>::operator[](size_t _index) const
	{
		return getRow(_index);
	}

	template <typename T>
	typename Matrix4x4Base<T>::RowVectorType& Matrix4x4Base<T>::operator[](size_t _index)
	{
		return getRow(_index);
	}

	template <typename T>
	const typename Matrix4x4Base<T>::RowVectorType& Matrix4x4Base<T>::getRow(size_t _index) const
	{
		EGO_ASSERT(_index < 4);
		return m_rows[_index];
	}

	template <typename T>
	typename Matrix4x4Base<T>::RowVectorType& Matrix4x4Base<T>::getRow(size_t _index)
	{
		EGO_ASSERT(_index < 4);
		return m_rows[_index];
	}

	template <typename T>
	typename Matrix4x4Base<T>::ValueType Matrix4x4Base<T>::getElement(size_t _row, size_t _column) const
	{
		EGO_ASSERT(_row < 4 && _column < 4);
		return m_values[_row][_column];
	}

	template <typename T>
	typename Matrix4x4Base<T>::ValueType& Matrix4x4Base<T>::getElement(size_t _row, size_t _column)
	{
		EGO_ASSERT(_row < 4 && _column < 4);
		return m_values[_row][_column];
	}

	template <typename T>
	void Matrix4x4Base<T>::setElement(size_t _row, size_t _column, ValueType _value)
	{
		getElement(_row, _column) = _value;
	}
#pragma endregion
}