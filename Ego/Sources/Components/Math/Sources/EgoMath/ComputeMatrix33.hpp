#pragma once

namespace ego
{
	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::operator=(const ComputeMatrix3x3Base<T>& _matrix)
	{
		m_matrix = _matrix.m_matrix;

		return *this;
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::operator+=(const ComputeMatrix3x3Base<T>& _matrix)
	{
		m_column1 += _matrix.m_column1;
		m_column2 += _matrix.m_column2;
		m_column3 += _matrix.m_column3;

		return *this;
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::operator-=(const ComputeMatrix3x3Base<T>& _matrix)
	{
		m_column1 -= _matrix.m_column1;
		m_column2 -= _matrix.m_column2;
		m_column3 -= _matrix.m_column3;

		return *this;
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::operator*=(const ComputeMatrix3x3Base<T>& _matrix)
	{
		ComputeMatrix3x3Base<T> result;

		result.m_matrix[0][0] = m_matrix[0][0] * _matrix.m_matrix[0][0] + m_matrix[1][0] * _matrix.m_matrix[0][1] + m_matrix[2][0] * _matrix.m_matrix[0][2];
		result.m_matrix[0][1] = m_matrix[0][1] * _matrix.m_matrix[0][0] + m_matrix[1][1] * _matrix.m_matrix[0][1] + m_matrix[2][1] * _matrix.m_matrix[0][2];
		result.m_matrix[0][2] = m_matrix[0][2] * _matrix.m_matrix[0][0] + m_matrix[1][2] * _matrix.m_matrix[0][1] + m_matrix[2][2] * _matrix.m_matrix[0][2];

		result.m_matrix[1][0] = m_matrix[0][0] * _matrix.m_matrix[1][0] + m_matrix[1][0] * _matrix.m_matrix[1][1] + m_matrix[2][0] * _matrix.m_matrix[1][2];
		result.m_matrix[1][1] = m_matrix[0][1] * _matrix.m_matrix[1][0] + m_matrix[1][1] * _matrix.m_matrix[1][1] + m_matrix[2][1] * _matrix.m_matrix[1][2];
		result.m_matrix[1][2] = m_matrix[0][2] * _matrix.m_matrix[1][0] + m_matrix[1][2] * _matrix.m_matrix[1][1] + m_matrix[2][2] * _matrix.m_matrix[1][2];

		result.m_matrix[2][0] = m_matrix[0][0] * _matrix.m_matrix[2][0] + m_matrix[1][0] * _matrix.m_matrix[2][1] + m_matrix[2][0] * _matrix.m_matrix[2][2];
		result.m_matrix[2][1] = m_matrix[0][1] * _matrix.m_matrix[2][0] + m_matrix[1][1] * _matrix.m_matrix[2][1] + m_matrix[2][1] * _matrix.m_matrix[2][2];
		result.m_matrix[2][2] = m_matrix[0][2] * _matrix.m_matrix[2][0] + m_matrix[1][2] * _matrix.m_matrix[2][1] + m_matrix[2][2] * _matrix.m_matrix[2][2];

		*this = result;
		return *this;
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::operator*=(T _value)
	{
		m_column1 *= _value;
		m_column2 *= _value;
		m_column3 *= _value;

		return *this;
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::operator/=(T _value)
	{
		m_column1 /= _value;
		m_column2 /= _value;
		m_column3 /= _value;

		return *this;
	}

	template <typename T>
	bool ComputeMatrix3x3Base<T>::operator==(const ComputeMatrix3x3Base<T>& _matrix) const
	{
		return isEqual(_matrix);
	}

	template <typename T>
	bool ComputeMatrix3x3Base<T>::operator!=(const ComputeMatrix3x3Base<T>& _matrix) const
	{
		return !isEqual(_matrix);
	}

	template <typename T>
	FloatMatrix3x3 ComputeMatrix3x3Base<T>::getFloatMatrix3x3() const
	{
		FloatMatrix3x3 result;
		getFloatMatrix3x3(result);

		return result;
	}

	template <typename T>
	const ComputeVector3Base<T>& ComputeMatrix3x3Base<T>::getColumn(uint32_t _index) const
	{
		EGO_ASSERT(_index < 3);
		return m_columns[_index];
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeMatrix3x3Base<T>::getColumn(uint32_t _index)
	{
		EGO_ASSERT(_index < 3);
		return m_columns[_index];
	}

	template <typename T>
	ComputeVector3Base<T> ComputeMatrix3x3Base<T>::getRow(uint32_t _index) const
	{
		return ComputeVector3(m_matrix.m_rows[0][_index], m_matrix.m_rows[1][_index], m_matrix.m_rows[2][_index]);
	}

	template <typename T>
	void ComputeMatrix3x3Base<T>::setColumn(uint32_t _index, const ComputeVector3Base<T>& _column)
	{
		EGO_ASSERT(_index < 3);
		m_columns[_index] = _column;
	}

	template <typename T>
	void ComputeMatrix3x3Base<T>::setRow(uint32_t _index, const ComputeVector3Base<T>& _row)
	{
		EGO_ASSERT(_index < 3);

		m_matrix.m_rows[0][_index] = _row.getX();
		m_matrix.m_rows[1][_index] = _row.getY();
		m_matrix.m_rows[2][_index] = _row.getZ();
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::negate()
	{
		m_column1 = -m_column1;
		m_column2 = -m_column2;
		m_column3 = -m_column3;

		return *this;
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::transpose()
	{
		ComputeMatrix3x3Base<T> transposed;

		transposed.setColumn(0, getRow(0));
		transposed.setColumn(1, getRow(1));
		transposed.setColumn(2, getRow(2));

		*this = transposed;
		return *this;
	}

	template <typename T>
	T ComputeMatrix3x3Base<T>::getDeterminant() const
	{
		return
			m_matrix[0][0] * (m_matrix[1][1] * m_matrix[2][2] - m_matrix[2][1] * m_matrix[1][2]) -
			m_matrix[0][1] * (m_matrix[1][0] * m_matrix[2][2] - m_matrix[2][0] * m_matrix[1][2]) +
			m_matrix[0][2] * (m_matrix[1][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[1][1]);
	}

	template <typename T>
	ComputeMatrix3x3Base<T>& ComputeMatrix3x3Base<T>::invert()
	{
		const ComputeValueType determinant = getDeterminant();

		EGO_ASSERT(abs(determinant) > math::Epsilon);

		ComputeVector3Base<T> column0(
			(m_matrix[1][1] * m_matrix[2][2] - m_matrix[2][1] * m_matrix[1][2]),
			-(m_matrix[0][1] * m_matrix[2][2] - m_matrix[2][1] * m_matrix[0][2]),
			(m_matrix[0][1] * m_matrix[1][2] - m_matrix[0][2] * m_matrix[1][1])
		);

		ComputeVector3Base<T> column1(
			-(m_matrix[1][0] * m_matrix[2][2] - m_matrix[2][0] * m_matrix[1][2]),
			(m_matrix[0][0] * m_matrix[2][2] - m_matrix[2][0] * m_matrix[0][2]),
			-(m_matrix[0][0] * m_matrix[1][2] - m_matrix[1][0] * m_matrix[0][2])
		);

		ComputeVector3Base<T> column2(
			(m_matrix[1][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[1][1]),
			-(m_matrix[0][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[0][1]),
			(m_matrix[0][0] * m_matrix[1][1] - m_matrix[0][1] * m_matrix[1][0])
		);

		ComputeMatrix3x3Base<T> inverted(column0, column1, column2);
		inverted *= T(1.0) / determinant;

		*this = inverted;
		return *this;
	}

	template <typename T>
	ComputeVector3Base<T> ComputeMatrix3x3Base<T>::transform(const ComputeVector3Base<T>& _vector) const
	{
		ComputeVector3 result;
		transform(_vector, result);

		return result;
	}

	template <typename T>
	void ComputeMatrix3x3Base<T>::transform(const ComputeVector3Base<T>& _vector, ComputeVector3Base<T>& _out) const
	{
		_out[0] = m_matrix[0][0] * _vector[0] + m_matrix[1][0] * _vector[1] + m_matrix[2][0] * _vector[2];
		_out[1] = m_matrix[0][1] * _vector[0] + m_matrix[1][1] * _vector[1] + m_matrix[2][1] * _vector[2];
		_out[2] = m_matrix[0][2] * _vector[0] + m_matrix[1][2] * _vector[1] + m_matrix[2][2] * _vector[2];
	}

	template <typename T>
	bool ComputeMatrix3x3Base<T>::isEqual(const ComputeMatrix3x3Base<T>& _matrix, T _epsilon) const
	{
		if (m_column1.isEqual(_matrix.m_column1, _epsilon)) { return false; }
		if (m_column2.isEqual(_matrix.m_column2, _epsilon)) { return false; }
		if (m_column3.isEqual(_matrix.m_column3, _epsilon)) { return false; }

		return true;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator+(const ComputeMatrix3x3Base<T>& _matrix1, const ComputeMatrix3x3Base<T>& _matrix2)
	{
		ComputeMatrix3x3Base<T> result(_matrix1);
		return result += _matrix2;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator-(const ComputeMatrix3x3Base<T>& _matrix1, const ComputeMatrix3x3Base<T>& _matrix2)
	{
		ComputeMatrix3x3Base<T> matrix3(_matrix1);
		return matrix3 -= _matrix2;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator-(const ComputeMatrix3x3Base<T>& _matrix)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result.negate();
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator*(const ComputeMatrix3x3Base<T>& _matrix1, const ComputeMatrix3x3Base<T>& _matrix2)
	{
		ComputeMatrix3x3Base<T> result(_matrix1);
		return result *= _matrix2;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator*(const ComputeMatrix3x3Base<T>& _matrix, T _value)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result *= _value;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator*(T _value, const ComputeMatrix3x3Base<T>& _matrix)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result *= _value;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator/(const ComputeMatrix3x3Base<T>& _matrix, T _value)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result /= _value;
	}

	template <typename T>
	ComputeMatrix3x3Base<T> operator/(T _value, const ComputeMatrix3x3Base<T>& _matrix)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result /= _value;
	}

	template <typename T>
	ComputeVector3Base<T> operator*(const ComputeMatrix3x3Base<T>& _matrix, const ComputeVector3Base<T>& _vector)
	{
		return _matrix.transform(_vector);
	}

	template <typename T>
	ComputeMatrix3x3Base<T> CrossProductSkewSymmetricComputeMatrix3x3(const ComputeVector3Base<T>& _vector)
	{
		return ComputeMatrix3x3Base<T>(
			T(0.0), _vector[2], -_vector[1],
			-_vector[2], T(0.0), _vector[0],
			_vector[1], -_vector[0], T(0.0)
		);
	}

	template <typename T>
	ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(const ComputeVector3Base<T>& _vector)
	{
		const T cx = cos(_vector.getX());
		const T sx = sin(_vector.getX());

		const T cy = cos(_vector.getY());
		const T sy = sin(_vector.getY());

		const T cz = cos(_vector.getZ());
		const T sz = sin(_vector.getZ());

		const ComputeVector3Base<T> column0(
			cz * cy + sz * sx * sy,
			sz * cx,
			sz * sx * cy - cz * sy
		);

		const ComputeVector3Base<T> column1(
			cz * sx * sy - sz * cy,
			cz * cx,
			sz * sy + cz * sx * cy
		);

		const ComputeVector3Base<T> column2(
			cx * sy,
			-sx,
			cx * cy
		);

		return ComputeMatrix3x3Base<T>(column0, column1, column2);
	}

	template <typename T>
	ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(T _roll, T _pitch, T _yaw)
	{
		return RotationEulerComputeMatrix3x3(ComputeVector3Base<T>(_roll, _pitch, _yaw));
	}

	template <typename T>
	ComputeVector3Base<T> EulerAnglesFromRotationComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _rotation)
	{
		if (abs(_rotation.getElement(2, 0)) < 1.0_ecv) {
			return ComputeVector3Base<T>(
				std::atan2(_rotation.getElement(2, 1), _rotation.getElement(2, 2)),
				-std::asin(_rotation.getElement(2, 0)),
				std::atan2(_rotation.getElement(1, 0), _rotation.getElement(0, 0))
			);
		}

		return ComputeVector3Base<T>(
			std::atan2(_rotation.getElement(0, 1), _rotation.getElement(1, 1)),
			(_rotation.getElement(2, 0) <= -1.0_ecv) ? math::HalfPi : -math::HalfPi,
			0.0_ecv
		);
	}

	template <typename T>
	ComputeMatrix3x3Base<T> ScaleComputeMatrix3x3(const ComputeVector3Base<T>& _scale)
	{
		return ComputeMatrix3x3Base<T>(
			ComputeVector3Base<T>(_scale.getX(), T(0.0), T(0.0)),
			ComputeVector3Base<T>(T(0.0), _scale.getY(), T(0.0)),
			ComputeVector3Base<T>(T(0.0), T(0.0), _scale.getZ())
		);
	}

	template <typename T>
	ComputeMatrix3x3Base<T> InvertComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result.invert();
	}

	template <typename T>
	ComputeMatrix3x3Base<T> TransposeComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix)
	{
		ComputeMatrix3x3Base<T> result(_matrix);
		return result.transpose();
	}
}