namespace ego
{
	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::operator=(const ComputeVector4Base<T>& _vector)
	{
		m_vector = _vector.m_vector;

		return *this;
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::operator+=(const ComputeVector4Base<T>& _vector)
	{
		m_vector.m_x += _vector.m_vector.m_x;
		m_vector.m_y += _vector.m_vector.m_y;
		m_vector.m_z += _vector.m_vector.m_z;
		m_vector.m_w += _vector.m_vector.m_w;

		return *this;
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::operator-=(const ComputeVector4Base<T>& _vector)
	{
		m_vector.m_x -= _vector.m_vector.m_x;
		m_vector.m_y -= _vector.m_vector.m_y;
		m_vector.m_z -= _vector.m_vector.m_z;
		m_vector.m_w -= _vector.m_vector.m_w;

		return *this;
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::operator*=(T _value)
	{
		m_vector.m_x *= _value;
		m_vector.m_y *= _value;
		m_vector.m_z *= _value;
		m_vector.m_w *= _value;

		return *this;
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::operator/=(T _value)
	{
		EGO_ASSERT(std::abs(_value) > math::Epsilon);

		m_vector.m_x /= _value;
		m_vector.m_y /= _value;
		m_vector.m_z /= _value;
		m_vector.m_w /= _value;

		return *this;
	}

	template <typename T>
	bool ComputeVector4Base<T>::operator==(const ComputeVector4Base<T>& _vector) const
	{
		return isEqual(_vector);
	}

	template <typename T>
	bool ComputeVector4Base<T>::operator!=(const ComputeVector4Base<T>& _vector) const
	{
		return !operator==(_vector);
	}

	template <typename T>
	FloatVector4 ComputeVector4Base<T>::getFloatVector4() const
	{
		FloatVector4 result;
		getFloatVector4(result);

		return result;
	}

	template <typename T>
	void ComputeVector4Base<T>::getFloatVector4(FloatVector4& _out) const
	{
		_out = FloatVector4(float(m_vector.m_x), float(m_vector.m_y), float(m_vector.m_z), float(m_vector.m_w));
	}

	template <typename T>
	T ComputeVector4Base<T>::getLength() const
	{
		return sqrt(getLengthSqr());
	}

	template <typename T>
	T ComputeVector4Base<T>::getLengthSqr() const
	{
		return dot(*this);
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::negate()
	{
		m_vector.m_x = -m_vector.m_x;
		m_vector.m_y = -m_vector.m_y;
		m_vector.m_z = -m_vector.m_z;
		m_vector.m_w = -m_vector.m_w;

		return *this;
	}

	template <typename T>
	ComputeVector4Base<T> ComputeVector4Base<T>::getUnit() const
	{
		ComputeVector4Base<T> _vector(*this);
		return _vector.normalize();
	}

	template <typename T>
	ComputeVector4Base<T>& ComputeVector4Base<T>::normalize()
	{
		const T length = getLength();
		if (length <= math::TypedEpsilon<T>())
		{
			return *this;
		}

		return operator/=(length);
	}

	template <typename T>
	T ComputeVector4Base<T>::dot(const ComputeVector4Base<T>& _vector) const
	{
		return
			m_vector.m_x * _vector.m_vector.m_x +
			m_vector.m_y * _vector.m_vector.m_y +
			m_vector.m_z * _vector.m_vector.m_z +
			m_vector.m_w * _vector.m_vector.m_w;
	}

	template <typename T>
	bool ComputeVector4Base<T>::isEqual(const ComputeVector4Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon) &&
			math::IsApproxEqual(m_vector.m_w, _vector.m_vector.m_w, _epsilon);
	}

	template <typename T>
	bool ComputeVector4Base<T>::isLess(const ComputeVector4Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxLess(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxLess(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxLess(m_vector.m_z, _vector.m_vector.m_z, _epsilon) &&
			math::IsApproxLess(m_vector.m_w, _vector.m_vector.m_w, _epsilon);
	}

	template <typename T>
	bool ComputeVector4Base<T>::isLessOrEqual(const ComputeVector4Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxLessOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxLessOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxLessOrEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon) &&
			math::IsApproxLessOrEqual(m_vector.m_w, _vector.m_vector.m_w, _epsilon);
	}

	template <typename T>
	bool ComputeVector4Base<T>::isGreater(const ComputeVector4Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxGreater(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxGreater(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxGreater(m_vector.m_z, _vector.m_vector.m_z, _epsilon) &&
			math::IsApproxGreater(m_vector.m_w, _vector.m_vector.m_w, _epsilon);
	}

	template <typename T>
	bool ComputeVector4Base<T>::isGreaterOrEqual(const ComputeVector4Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxGreaterOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxGreaterOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxGreaterOrEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon) &&
			math::IsApproxGreaterOrEqual(m_vector.m_w, _vector.m_vector.m_w, _epsilon);
	}

	template <typename T>
	bool ComputeVector4Base<T>::isUnit() const
	{
		return math::IsApproxEqual(getLengthSqr(), T(1.0));
	}

	template <typename T>
	bool ComputeVector4Base<T>::isZero() const
	{
		return math::IsApproxEqual(getLengthSqr(), T(0.0));
	}

	template <typename T>
	ComputeVector4Base<T> operator+(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2)
	{
		ComputeVector4Base<T> result(_vector1);
		return result += _vector2;
	}

	template <typename T>
	ComputeVector4Base<T> operator-(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2)
	{
		ComputeVector4Base<T> result(_vector1);
		return result -= _vector2;
	}

	template <typename T>
	ComputeVector4Base<T> operator-(const ComputeVector4Base<T>& _vector)
	{
		ComputeVector4Base<T> result(_vector);
		return result.negate();
	}

	template <typename T>
	ComputeVector4Base<T> operator*(const ComputeVector4Base<T>& _vector, T _value)
	{
		ComputeVector4Base<T> result(_vector);
		return result *= _value;
	}

	template <typename T>
	ComputeVector4Base<T> operator*(T _value, const ComputeVector4Base<T>& _vector)
	{
		return operator*(_vector, _value);
	}

	template <typename T>
	ComputeVector4Base<T> operator/(const ComputeVector4Base<T>& _vector, T _value)
	{
		ComputeVector4Base<T> result(_vector);
		return result /= _value;
	}

	template <typename T>
	ComputeVector4Base<T> operator/(T _value, const ComputeVector4Base<T>& _vector)
	{
		return operator/(_vector, _value);
	}

	template <typename T>
	ComputeVector4Base<T> ComputeVector4FromPoint(const ComputeVector3Base<T>& _vector)
	{
		return ComputeVector4Base<T>(_vector, T(1.0));
	}

	template <typename T>
	T DotComputeVector4(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2)
	{
		ComputeVector4Base<T> result(_vector1);
		return result.dot(_vector2);
	}
}