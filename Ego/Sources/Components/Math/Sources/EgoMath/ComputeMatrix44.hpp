#pragma once

namespace ego
{
	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::operator=(const ComputeMatrix4x4Base<T>& _matrix)
	{
		m_matrix = _matrix.m_matrix;

		return *this;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::operator+=(const ComputeMatrix4x4Base<T>& _matrix)
	{
		m_column1 += _matrix.m_column1;
		m_column2 += _matrix.m_column2;
		m_column3 += _matrix.m_column3;
		m_column4 += _matrix.m_column4;

		return *this;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::operator-=(const ComputeMatrix4x4Base<T>& _matrix)
	{
		m_column1 -= _matrix.m_column1;
		m_column2 -= _matrix.m_column2;
		m_column3 -= _matrix.m_column3;
		m_column4 -= _matrix.m_column4;

		return *this;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::operator*=(const ComputeMatrix4x4Base<T>& _matrix)
	{
		ComputeMatrix4x4Base<T> result;

		result.m_matrix[0][0] = m_matrix[0][0] * _matrix.m_matrix[0][0] + m_matrix[1][0] * _matrix.m_matrix[0][1] + m_matrix[2][0] * _matrix.m_matrix[0][2] + m_matrix[3][0] * _matrix.m_matrix[0][3];
		result.m_matrix[0][1] = m_matrix[0][1] * _matrix.m_matrix[0][0] + m_matrix[1][1] * _matrix.m_matrix[0][1] + m_matrix[2][1] * _matrix.m_matrix[0][2] + m_matrix[3][1] * _matrix.m_matrix[0][3];
		result.m_matrix[0][2] = m_matrix[0][2] * _matrix.m_matrix[0][0] + m_matrix[1][2] * _matrix.m_matrix[0][1] + m_matrix[2][2] * _matrix.m_matrix[0][2] + m_matrix[3][2] * _matrix.m_matrix[0][3];
		result.m_matrix[0][3] = m_matrix[0][3] * _matrix.m_matrix[0][0] + m_matrix[1][3] * _matrix.m_matrix[0][1] + m_matrix[2][3] * _matrix.m_matrix[0][2] + m_matrix[3][3] * _matrix.m_matrix[0][3];

		result.m_matrix[1][0] = m_matrix[0][0] * _matrix.m_matrix[1][0] + m_matrix[1][0] * _matrix.m_matrix[1][1] + m_matrix[2][0] * _matrix.m_matrix[1][2] + m_matrix[3][0] * _matrix.m_matrix[1][3];
		result.m_matrix[1][1] = m_matrix[0][1] * _matrix.m_matrix[1][0] + m_matrix[1][1] * _matrix.m_matrix[1][1] + m_matrix[2][1] * _matrix.m_matrix[1][2] + m_matrix[3][1] * _matrix.m_matrix[1][3];
		result.m_matrix[1][2] = m_matrix[0][2] * _matrix.m_matrix[1][0] + m_matrix[1][2] * _matrix.m_matrix[1][1] + m_matrix[2][2] * _matrix.m_matrix[1][2] + m_matrix[3][2] * _matrix.m_matrix[1][3];
		result.m_matrix[1][3] = m_matrix[0][3] * _matrix.m_matrix[1][0] + m_matrix[1][3] * _matrix.m_matrix[1][1] + m_matrix[2][3] * _matrix.m_matrix[1][2] + m_matrix[3][3] * _matrix.m_matrix[1][3];

		result.m_matrix[2][0] = m_matrix[0][0] * _matrix.m_matrix[2][0] + m_matrix[1][0] * _matrix.m_matrix[2][1] + m_matrix[2][0] * _matrix.m_matrix[2][2] + m_matrix[3][0] * _matrix.m_matrix[2][3];
		result.m_matrix[2][1] = m_matrix[0][1] * _matrix.m_matrix[2][0] + m_matrix[1][1] * _matrix.m_matrix[2][1] + m_matrix[2][1] * _matrix.m_matrix[2][2] + m_matrix[3][1] * _matrix.m_matrix[2][3];
		result.m_matrix[2][2] = m_matrix[0][2] * _matrix.m_matrix[2][0] + m_matrix[1][2] * _matrix.m_matrix[2][1] + m_matrix[2][2] * _matrix.m_matrix[2][2] + m_matrix[3][2] * _matrix.m_matrix[2][3];
		result.m_matrix[2][3] = m_matrix[0][3] * _matrix.m_matrix[2][0] + m_matrix[1][3] * _matrix.m_matrix[2][1] + m_matrix[2][3] * _matrix.m_matrix[2][2] + m_matrix[3][3] * _matrix.m_matrix[2][3];

		result.m_matrix[3][0] = m_matrix[0][0] * _matrix.m_matrix[3][0] + m_matrix[1][0] * _matrix.m_matrix[3][1] + m_matrix[2][0] * _matrix.m_matrix[3][2] + m_matrix[3][0] * _matrix.m_matrix[3][3];
		result.m_matrix[3][1] = m_matrix[0][1] * _matrix.m_matrix[3][0] + m_matrix[1][1] * _matrix.m_matrix[3][1] + m_matrix[2][1] * _matrix.m_matrix[3][2] + m_matrix[3][1] * _matrix.m_matrix[3][3];
		result.m_matrix[3][2] = m_matrix[0][2] * _matrix.m_matrix[3][0] + m_matrix[1][2] * _matrix.m_matrix[3][1] + m_matrix[2][2] * _matrix.m_matrix[3][2] + m_matrix[3][2] * _matrix.m_matrix[3][3];
		result.m_matrix[3][3] = m_matrix[0][3] * _matrix.m_matrix[3][0] + m_matrix[1][3] * _matrix.m_matrix[3][1] + m_matrix[2][3] * _matrix.m_matrix[3][2] + m_matrix[3][3] * _matrix.m_matrix[3][3];

		*this = result;
		return *this;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::operator*=(T _value)
	{
		m_column1 *= _value;
		m_column2 *= _value;
		m_column3 *= _value;
		m_column4 *= _value;

		return *this;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::operator/=(T _value)
	{
		m_column1 /= _value;
		m_column2 /= _value;
		m_column3 /= _value;
		m_column4 /= _value;

		return *this;
	}

	template <typename T>
	bool ComputeMatrix4x4Base<T>::operator==(const ComputeMatrix4x4Base<T>& _matrix) const
	{
		return isEqual(_matrix);
	}

	template <typename T>
	bool ComputeMatrix4x4Base<T>::operator!=(const ComputeMatrix4x4Base<T>& _matrix) const
	{
		return !isEqual(_matrix);
	}

	template <typename T>
	FloatMatrix4x4 ComputeMatrix4x4Base<T>::getFloatMatrix4x4() const
	{
		FloatMatrix4x4 result;
		getFloatMatrix4x4(result);

		return result;
	}

	template <typename T>
	const ComputeVector4Base<T>& ComputeMatrix4x4Base<T>::getColumn(uint32_t _index) const
	{
		EGO_ASSERT(_index < 4);
		return m_columns[_index];
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeMatrix4x4Base<T>::getColumn(uint32_t _index)
	{
		EGO_ASSERT(_index < 4);
		return m_columns[_index];
	}

	template <typename T>
	ComputeVector4Base<T> ComputeMatrix4x4Base<T>::getRow(uint32_t _index) const
	{
		return ComputeVector4Base<T>(m_matrix.m_rows[0][_index], m_matrix.m_rows[1][_index], m_matrix.m_rows[2][_index], m_matrix.m_rows[3][_index]);
	}

	template <typename T>
	void ComputeMatrix4x4Base<T>::setColumn(uint32_t _index, const ComputeVector4Base<T>& _column)
	{
		EGO_ASSERT(_index < 4);
		m_columns[_index] = _column;
	}

	template <typename T>
	void ComputeMatrix4x4Base<T>::setRow(uint32_t _index, const ComputeVector4Base<T>& _row)
	{
		EGO_ASSERT(_index < 4);

		m_matrix.m_rows[0][_index] = _row.getX();
		m_matrix.m_rows[1][_index] = _row.getY();
		m_matrix.m_rows[2][_index] = _row.getZ();
		m_matrix.m_rows[3][_index] = _row.getW();
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::negate()
	{
		m_column1 = -m_column1;
		m_column2 = -m_column2;
		m_column3 = -m_column3;
		m_column4 = -m_column4;

		return *this;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::transpose()
	{
		ComputeMatrix4x4Base<T> transposed;

		transposed.setColumn(0, getRow(0));
		transposed.setColumn(1, getRow(1));
		transposed.setColumn(2, getRow(2));
		transposed.setColumn(3, getRow(3));

		*this = transposed;
		return *this;
	}

	template <typename T>
	T ComputeMatrix4x4Base<T>::getDeterminant() const
	{
		const T det0 =
			m_matrix[1][1] * (m_matrix[2][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[2][3]) -
			m_matrix[2][1] * (m_matrix[1][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[1][3]) +
			m_matrix[3][1] * (m_matrix[1][2] * m_matrix[2][3] - m_matrix[2][2] * m_matrix[1][3]);

		const T det1 =
			m_matrix[0][1] * (m_matrix[2][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[2][3]) -
			m_matrix[2][1] * (m_matrix[0][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[0][3]) +
			m_matrix[3][1] * (m_matrix[0][2] * m_matrix[2][3] - m_matrix[2][2] * m_matrix[0][3]);

		const T det2 =
			m_matrix[0][1] * (m_matrix[1][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[1][3]) -
			m_matrix[1][1] * (m_matrix[0][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[0][3]) +
			m_matrix[3][1] * (m_matrix[0][2] * m_matrix[1][3] - m_matrix[1][2] * m_matrix[0][3]);

		const T det3 =
			m_matrix[0][1] * (m_matrix[1][2] * m_matrix[2][3] - m_matrix[2][2] * m_matrix[1][3]) -
			m_matrix[1][1] * (m_matrix[0][2] * m_matrix[2][3] - m_matrix[2][2] * m_matrix[0][3]) +
			m_matrix[2][1] * (m_matrix[0][2] * m_matrix[1][3] - m_matrix[1][2] * m_matrix[0][3]);

		return m_matrix[0][0] * det0 - m_matrix[1][0] * det1 + m_matrix[2][0] * det2 - m_matrix[3][0] * det3;
	}

	template <typename T>
	ComputeMatrix4x4Base<T>& ComputeMatrix4x4Base<T>::invert()
	{
		const T s0 = m_matrix[0][0] * m_matrix[1][1] - m_matrix[1][0] * m_matrix[0][1];
		const T s1 = m_matrix[0][0] * m_matrix[1][2] - m_matrix[1][0] * m_matrix[0][2];
		const T s2 = m_matrix[0][0] * m_matrix[1][3] - m_matrix[1][0] * m_matrix[0][3];
		const T s3 = m_matrix[0][1] * m_matrix[1][2] - m_matrix[1][1] * m_matrix[0][2];
		const T s4 = m_matrix[0][1] * m_matrix[1][3] - m_matrix[1][1] * m_matrix[0][3];
		const T s5 = m_matrix[0][2] * m_matrix[1][3] - m_matrix[1][2] * m_matrix[0][3];

		const T c5 = m_matrix[2][2] * m_matrix[3][3] - m_matrix[3][2] * m_matrix[2][3];
		const T c4 = m_matrix[2][1] * m_matrix[3][3] - m_matrix[3][1] * m_matrix[2][3];
		const T c3 = m_matrix[2][1] * m_matrix[3][2] - m_matrix[3][1] * m_matrix[2][2];
		const T c2 = m_matrix[2][0] * m_matrix[3][3] - m_matrix[3][0] * m_matrix[2][3];
		const T c1 = m_matrix[2][0] * m_matrix[3][2] - m_matrix[3][0] * m_matrix[2][2];
		const T c0 = m_matrix[2][0] * m_matrix[3][1] - m_matrix[3][0] * m_matrix[2][1];

		const T determinant = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
		EGO_ASSERT(abs(determinant) > math::Epsilon);

		ComputeVector4Base<T> column0(
			m_matrix[1][1] * c5 - m_matrix[1][2] * c4 + m_matrix[1][3] * c3,
			-m_matrix[0][1] * c5 + m_matrix[0][2] * c4 - m_matrix[0][3] * c3,
			m_matrix[3][1] * s5 - m_matrix[3][2] * s4 + m_matrix[3][3] * s3,
			-m_matrix[2][1] * s5 + m_matrix[2][2] * s4 - m_matrix[2][3] * s3
		);

		ComputeVector4Base<T> column1(
			-m_matrix[1][0] * c5 + m_matrix[1][2] * c2 - m_matrix[1][3] * c1,
			m_matrix[0][0] * c5 - m_matrix[0][2] * c2 + m_matrix[0][3] * c1,
			-m_matrix[3][0] * s5 + m_matrix[3][2] * s2 - m_matrix[3][3] * s1,
			m_matrix[2][0] * s5 - m_matrix[2][2] * s2 + m_matrix[2][3] * s1
		);

		ComputeVector4Base<T> column2(
			m_matrix[1][0] * c4 - m_matrix[1][1] * c2 + m_matrix[1][3] * c0,
			-m_matrix[0][0] * c4 + m_matrix[0][1] * c2 - m_matrix[0][3] * c0,
			m_matrix[3][0] * s4 - m_matrix[3][1] * s2 + m_matrix[3][3] * s0,
			-m_matrix[2][0] * s4 + m_matrix[2][1] * s2 - m_matrix[2][3] * s0
		);

		ComputeVector4Base<T> column3(
			-m_matrix[1][0] * c3 + m_matrix[1][1] * c1 - m_matrix[1][2] * c0,
			m_matrix[0][0] * c3 - m_matrix[0][1] * c1 + m_matrix[0][2] * c0,
			-m_matrix[3][0] * s3 + m_matrix[3][1] * s1 - m_matrix[3][2] * s0,
			m_matrix[2][0] * s3 - m_matrix[2][1] * s1 + m_matrix[2][2] * s0
		);

		ComputeMatrix4x4Base<T> inverted(column0, column1, column2, column3);
		inverted *= T(1.0) / determinant;

		*this = inverted;
		return *this;
	}

	template <typename T>
	ComputeVector4Base<T> ComputeMatrix4x4Base<T>::transform(const ComputeVector4Base<T>& _vector) const
	{
		ComputeVector4Base<T> result;
		transform(_vector, result);

		return result;
	}

	template <typename T>
	void ComputeMatrix4x4Base<T>::transform(const ComputeVector4Base<T>& _vector, ComputeVector4Base<T>& _out) const
	{
		_out[0] = m_matrix[0][0] * _vector[0] + m_matrix[1][0] * _vector[1] + m_matrix[2][0] * _vector[2] + m_matrix[3][0] * _vector[3];
		_out[1] = m_matrix[0][1] * _vector[0] + m_matrix[1][1] * _vector[1] + m_matrix[2][1] * _vector[2] + m_matrix[3][1] * _vector[3];
		_out[2] = m_matrix[0][2] * _vector[0] + m_matrix[1][2] * _vector[1] + m_matrix[2][2] * _vector[2] + m_matrix[3][2] * _vector[3];
		_out[3] = m_matrix[0][3] * _vector[0] + m_matrix[1][3] * _vector[1] + m_matrix[2][3] * _vector[2] + m_matrix[3][3] * _vector[3];
	}

	template <typename T>
	bool ComputeMatrix4x4Base<T>::isEqual(const ComputeMatrix4x4Base<T>& _matrix, T _epsilon) const
	{
		if (m_column1.isEqual(_matrix.m_column1, _epsilon)) { return false; }
		if (m_column2.isEqual(_matrix.m_column2, _epsilon)) { return false; }
		if (m_column3.isEqual(_matrix.m_column3, _epsilon)) { return false; }
		if (m_column4.isEqual(_matrix.m_column4, _epsilon)) { return false; }

		return true;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator+(const ComputeMatrix4x4Base<T>& _matrix1, const ComputeMatrix4x4Base<T>& _matrix2)
	{
		ComputeMatrix4x4Base<T> result(_matrix1);
		return result += _matrix2;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator-(const ComputeMatrix4x4Base<T>& _matrix1, const ComputeMatrix4x4Base<T>& _matrix2)
	{
		ComputeMatrix4x4Base<T> result(_matrix1);
		return result -= _matrix2;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator-(const ComputeMatrix4x4Base<T>& _matrix)
	{
		ComputeMatrix4x4Base<T> result(_matrix);
		return result.negate();
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator*(const ComputeMatrix4x4Base<T>& _matrix1, const ComputeMatrix4x4Base<T>& _matrix2)
	{
		ComputeMatrix4x4Base<T> result(_matrix1);
		return result *= _matrix2;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator*(const ComputeMatrix4x4Base<T>& _matrix, T _value)
	{
		ComputeMatrix4x4Base<T> result(_matrix);
		return result *= _value;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator*(T _value, const ComputeMatrix4x4Base<T>& _matrix)
	{
		ComputeMatrix4x4Base<T> result(_matrix);
		return result *= _value;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator/(const ComputeMatrix4x4Base<T>& _matrix, T _value)
	{
		ComputeMatrix4x4Base<T> result(_matrix);
		return result /= _value;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> operator/(T _value, const ComputeMatrix4x4Base<T>& _matrix)
	{
		ComputeMatrix4x4Base<T> result(_matrix);
		return result /= _value;
	}

	template <typename T>
	ComputeVector4Base<T> operator*(const ComputeMatrix4x4Base<T>& _matrix, const ComputeVector4Base<T>& _vector)
	{
		return _matrix.transform(_vector);
	}

	template <typename T>
	ComputeMatrix4x4Base<T> CrossProductSkewSymmetricComputeMatrix4x4(const ComputeVector4Base<T>& _vector)
	{
		return ComputeMatrix4x4Base<T>(
			T(0.0), _vector[2], -_vector[1], T(0.0),
			-_vector[2], T(0.0), _vector[0], T(0.0),
			_vector[1], -_vector[0], T(0.0), T(0.0),
			T(0.0), T(0.0), T(0.0), T(1.0)
		);
	}

	template <typename T>
	ComputeMatrix4x4Base<T> TranslationComputeMatrix4x4(const ComputeVector3Base<T>& _vector)
	{
		ComputeMatrix4x4Base<T> result = ComputeMatrix4x4IdentityBase<T>();
		result.setColumn(3, ComputeVector4Base<T>(_vector, T(1.0)));

		return result;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(const ComputeVector3Base<T>& _vector)
	{
		ComputeMatrix4x4Base<T> result = RotationEulerComputeMatrix3x3(_vector);
		result.setElement(3, 3, T(1.0));

		return result;
	}

	template <typename T>
	ComputeMatrix4x4Base<T> ScaleComputeMatrix4x4(const ComputeVector3Base<T>& _scale)
	{
		return ComputeMatrix4x4Base<T>(
			ComputeVector4Base<T>(_scale.getX(), T(0.0), T(0.0), T(0.0)),
			ComputeVector4Base<T>(T(0.0), _scale.getY(), T(0.0), T(0.0)),
			ComputeVector4Base<T>(T(0.0), T(0.0), _scale.getZ(), T(0.0)),
			ComputeVector4Base<T>(T(0.0), T(0.0), T(0.0), T(1.0))
		);
	}

	template <typename T>
	ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(T _pitch, T _yaw, T _roll)
	{
		return RotationEulerComputeMatrix4x4(ComputeVector3Base<T>(_pitch, _yaw, _roll));
	}

	template <typename T>
	ComputeMatrix4x4Base<T> InvertComputeMatrix4x4(const ComputeMatrix4x4Base<T>& _matrix)
	{
		ComputeMatrix4x4Base<T> result = _matrix;
		return result.invert();
	}
}