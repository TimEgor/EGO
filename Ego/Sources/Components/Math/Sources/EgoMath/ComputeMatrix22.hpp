    #pragma once

namespace ego
{
	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::operator=(const ComputeMatrix2x2Base<T>& _matrix)
	{
		m_matrix = _matrix.m_matrix;

		return *this;
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::operator+=(const ComputeMatrix2x2Base<T>& _matrix)
	{
		m_column1 += _matrix.m_column1;
		m_column2 += _matrix.m_column2;

		return *this;
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::operator-=(const ComputeMatrix2x2Base<T>& _matrix)
	{
		m_column1 -= _matrix.m_column1;
		m_column2 -= _matrix.m_column2;

		return *this;
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::operator*=(const ComputeMatrix2x2Base<T>& _matrix)
	{
		ComputeMatrix2x2Base<T> result;

		result.m_matrix[0][0] = m_matrix[0][0] * _matrix.m_matrix[0][0] + m_matrix[1][0] * _matrix.m_matrix[0][1];
		result.m_matrix[0][1] = m_matrix[0][1] * _matrix.m_matrix[0][0] + m_matrix[1][1] * _matrix.m_matrix[0][1];

		result.m_matrix[1][0] = m_matrix[0][0] * _matrix.m_matrix[1][0] + m_matrix[1][0] * _matrix.m_matrix[1][1];
		result.m_matrix[1][1] = m_matrix[0][1] * _matrix.m_matrix[1][0] + m_matrix[1][1] * _matrix.m_matrix[1][1];

		*this = result;
		return *this;
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::operator*=(T _value)
	{
		m_column1 *= _value;
		m_column2 *= _value;

		return *this;
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::operator/=(T _value)
	{
		m_column1 /= _value;
		m_column2 /= _value;

		return *this;
	}

	template <typename T>
	bool ComputeMatrix2x2Base<T>::operator==(const ComputeMatrix2x2Base<T>& _matrix) const
	{
		return isEqual(_matrix);
	}

	template <typename T>
	bool ComputeMatrix2x2Base<T>::operator!=(const ComputeMatrix2x2Base<T>& _matrix) const
	{
		return !isEqual(_matrix);
	}

	template <typename T>
	FloatMatrix2x2 ComputeMatrix2x2Base<T>::getFloatMatrix2x2() const
	{
		FloatMatrix2x2 result;
		getFloatMatrix2x2(result);

		return result;
	}

	template <typename T>
	const ComputeVector2Base<T>& ComputeMatrix2x2Base<T>::getColumn(uint32_t _index) const
	{
		EGO_ASSERT(_index < 2);
		return m_columns[_index];
	}

	template <typename T>
	ComputeVector2Base<T>& ComputeMatrix2x2Base<T>::getColumn(uint32_t _index)
	{
		EGO_ASSERT(_index < 2);
		return m_columns[_index];
	}

	template <typename T>
	ComputeVector2Base<T> ComputeMatrix2x2Base<T>::getRow(uint32_t _index) const
	{
		return ComputeVector2(m_matrix.m_rows[0][_index], m_matrix.m_rows[1][_index]);
	}

	template <typename T>
	void ComputeMatrix2x2Base<T>::setColumn(uint32_t _index, const ComputeVector2Base<T>& _column)
	{
		EGO_ASSERT(_index < 2);
		m_columns[_index] = _column;
	}

	template <typename T>
	void ComputeMatrix2x2Base<T>::setRow(uint32_t _index, const ComputeVector2Base<T>& _row)
	{
		EGO_ASSERT(_index < 3);

		m_matrix.m_rows[0][_index] = _row.getX();
		m_matrix.m_rows[1][_index] = _row.getY();
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::negate()
	{
		m_column1 = -m_column1;
		m_column2 = -m_column2;

		return *this;
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::transpose()
	{
		ComputeMatrix2x2Base<T> transposed;

		transposed.setColumn(0, getRow(0));
		transposed.setColumn(1, getRow(1));

		*this = transposed;
		return *this;
	}

	template <typename T>
	T ComputeMatrix2x2Base<T>::getDeterminant() const
	{
		return m_matrix[0][0] * m_matrix[1][1] - m_matrix[1][0] * m_matrix[0][1];
	}

	template <typename T>
	ComputeMatrix2x2Base<T>& ComputeMatrix2x2Base<T>::invert()
	{
		const ComputeValueType determinant = getDeterminant();

		EGO_ASSERT(abs(determinant) > math::Epsilon);

		ComputeMatrix2x2Base<T> inverted(
			m_matrix[1][1], -m_matrix[0][1],
			-m_matrix[1][0], m_matrix[0][0]
		);

		inverted *= ComputeValueType(1.0) / determinant;

		*this = inverted;
		return *this;
	}

	template <typename T>
	ComputeVector2Base<T> ComputeMatrix2x2Base<T>::transform(const ComputeVector2Base<T>& _vector) const
	{
		ComputeVector2 result;
		transform(_vector, result);

		return result;
	}

	template <typename T>
	void ComputeMatrix2x2Base<T>::transform(const ComputeVector2Base<T>& _vector, ComputeVector2Base<T>& _out) const
	{
		_out[0] = m_matrix[0][0] * _vector[0] + m_matrix[1][0] * _vector[1];
		_out[1] = m_matrix[0][1] * _vector[0] + m_matrix[1][1] * _vector[1];
	}

	template <typename T>
	bool ComputeMatrix2x2Base<T>::isEqual(const ComputeMatrix2x2Base<T>& _matrix, T _epsilon) const
	{
		if (m_column1.isEqual(_matrix.m_column1, _epsilon)) { return false; }
		if (m_column2.isEqual(_matrix.m_column2, _epsilon)) { return false; }

		return true;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator+(const ComputeMatrix2x2Base<T>& _matrix1, const ComputeMatrix2x2Base<T>& _matrix2)
	{
		ComputeMatrix2x2Base<T> result(_matrix1);
		return result += _matrix2;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator-(const ComputeMatrix2x2Base<T>& _matrix1, const ComputeMatrix2x2Base<T>& _matrix2)
	{
		ComputeMatrix2x2Base<T> result(_matrix1);
		return result -= _matrix2;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator-(const ComputeMatrix2x2Base<T>& _matrix)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result.negate();
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator*(const ComputeMatrix2x2Base<T>& _matrix1, const ComputeMatrix2x2Base<T>& _matrix2)
	{
		ComputeMatrix2x2Base<T> result(_matrix1);
		return result *= _matrix2;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator*(const ComputeMatrix2x2Base<T>& _matrix, T _value)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result *= _value;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator*(T _value, const ComputeMatrix2x2Base<T>& _matrix)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result *= _value;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator/(const ComputeMatrix2x2Base<T>& _matrix, T _value)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result /= _value;
	}

	template <typename T>
	ComputeMatrix2x2Base<T> operator/(T _value, const ComputeMatrix2x2Base<T>& _matrix)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result /= _value;
	}

	template <typename T>
	ComputeVector2Base<T> operator*(const ComputeMatrix2x2Base<T>& _matrix, const ComputeVector2Base<T>& _vector)
	{
		return _matrix.transform(_vector);
	}

	template <typename T>
	ComputeMatrix2x2Base<T> InvertComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result.invert();
	}

	template <typename T>
	ComputeMatrix2x2Base<T> TransposeComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix)
	{
		ComputeMatrix2x2Base<T> result(_matrix);
		return result.transpose();
	}
}