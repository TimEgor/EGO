namespace ego
{
	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::operator=(const ComputeVector3Base<T>& _vector)
	{
		m_vector = _vector.m_vector;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::operator+=(const ComputeVector3Base<T>& _vector)
	{
		m_vector.m_x += _vector.m_vector.m_x;
		m_vector.m_y += _vector.m_vector.m_y;
		m_vector.m_z += _vector.m_vector.m_z;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::operator-=(const ComputeVector3Base<T>& _vector)
	{
		m_vector.m_x -= _vector.m_vector.m_x;
		m_vector.m_y -= _vector.m_vector.m_y;
		m_vector.m_z -= _vector.m_vector.m_z;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::operator*=(T _value)
	{
		m_vector.m_x *= _value;
		m_vector.m_y *= _value;
		m_vector.m_z *= _value;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::operator/=(T _value)
	{
		EGO_ASSERT(std::abs(_value) > math::TypedEpsilon<T>());

		m_vector.m_x /= _value;
		m_vector.m_y /= _value;
		m_vector.m_z /= _value;

		return *this;
	}

	template <typename T>
	bool ComputeVector3Base<T>::operator==(const ComputeVector3Base<T>& _vector) const
	{
		return isEqual(_vector);
	}

	template <typename T>
	bool ComputeVector3Base<T>::operator!=(const ComputeVector3Base<T>& _vector) const
	{
		return !operator==(_vector);
	}

	template <typename T>
	FloatVector3 ComputeVector3Base<T>::getFloatVector3() const
	{
		FloatVector3 result;
		getFloatVector3(result);

		return result;
	}

	template <typename T>
	void ComputeVector3Base<T>::getFloatVector3(FloatVector3& _out) const
	{
		_out = FloatVector3(float(m_vector.m_x), float(m_vector.m_y), float(m_vector.m_z));
	}

	template <typename T>
	T ComputeVector3Base<T>::getLength() const
	{
		return sqrt(getLengthSqr());
	}

	template <typename T>
	T ComputeVector3Base<T>::getLengthSqr() const
	{
		return dot(*this);
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::negate()
	{
		m_vector.m_x = -m_vector.m_x;
		m_vector.m_y = -m_vector.m_y;
		m_vector.m_z = -m_vector.m_z;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::computeReciprocal()
	{
		m_vector.m_x = T(1.0) / m_vector.m_x;
		m_vector.m_y = T(1.0) / m_vector.m_y;
		m_vector.m_z = T(1.0) / m_vector.m_z;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::multiplyElements(const ComputeVector3Base<T>& _vector)
	{
		m_vector.m_x *= _vector.m_vector.m_x;
		m_vector.m_y *= _vector.m_vector.m_y;
		m_vector.m_z *= _vector.m_vector.m_z;

		return *this;
	}

	template <typename T>
	ComputeVector3Base<T> ComputeVector3Base<T>::getUnit() const
	{
		ComputeVector3Base<T> result;
		getUnit(result);

		return result;
	}

	template <typename T>
	void ComputeVector3Base<T>::getUnit(ComputeVector3Base<T>& _out) const
	{
		ComputeVector3Base<T> result(*this);
		_out = result.normalize();
	}

	template <typename T>
	ComputeVector3Base<T>& ComputeVector3Base<T>::normalize()
	{
		const T length = getLength();
		if (length <= math::TypedEpsilon<T>())
		{
			return *this;
		}

		return operator/=(length);
	}

	template <typename T>
	ComputeVector3Base<T> ComputeVector3Base<T>::getOrthogonal() const
	{
		ComputeVector3Base<T> result;
		getOrthogonal(result);

		return result;
	}

	template <typename T>
	void ComputeVector3Base<T>::getOrthogonal(ComputeVector3Base<T>& _out) const
	{
		if (abs(m_vector.m_x) > abs(m_vector.m_y))
		{
			_out.m_vector.m_x = m_vector.m_z;
			_out.m_vector.m_y = T(0.0);
			_out.m_vector.m_z = -m_vector.m_x;
		}
		else
		{
			_out.m_vector.m_x = T(0.0);
			_out.m_vector.m_y = m_vector.m_z;
			_out.m_vector.m_z = -m_vector.m_y;
		}
	}

	template <typename T>
	ComputeVector3Base<T> ComputeVector3Base<T>::getAbs() const
	{
		ComputeVector3Base<T> result;
		getAbs(result);

		return result;
	}

	template <typename T>
	void ComputeVector3Base<T>::getAbs(ComputeVector3Base<T>& _out) const
	{
		_out.m_vector.m_x = abs(m_vector.m_x);
		_out.m_vector.m_y = abs(m_vector.m_y);
		_out.m_vector.m_z = abs(m_vector.m_z);
	}

	template <typename T>
	uint32_t ComputeVector3Base<T>::getMinElementIndex() const
	{
		return
			getX() < getY() ?
			getX() < getZ() ? 0 : 2 :
			getY() < getZ() ? 1 : 2;
	}

	template <typename T>
	uint32_t ComputeVector3Base<T>::getMaxElementIndex() const
	{
		return
			getX() > getY() ?
			getX() > getZ() ? 0 : 2 :
			getY() > getZ() ? 1 : 2;
	}

	template <typename T>
	T ComputeVector3Base<T>::dot(const ComputeVector3Base<T>& _vector) const
	{
		return
			m_vector.m_x * _vector.m_vector.m_x +
			m_vector.m_y * _vector.m_vector.m_y +
			m_vector.m_z * _vector.m_vector.m_z;
	}

	template <typename T>
	ComputeVector3Base<T> ComputeVector3Base<T>::cross(const ComputeVector3Base<T>& _vector) const
	{
		return ComputeVector3Base<T>(
			m_vector.m_y * _vector.m_vector.m_z - m_vector.m_z * _vector.m_vector.m_y,
			m_vector.m_z * _vector.m_vector.m_x - m_vector.m_x * _vector.m_vector.m_z,
			m_vector.m_x * _vector.m_vector.m_y - m_vector.m_y * _vector.m_vector.m_x);
	}

	template <typename T>
	ComputeVector3Base<T> ComputeVector3Base<T>::swizzle(uint32_t _index1, uint32_t _index2, uint32_t _index3) const
	{
		return ComputeVector3Base<T>(getElement(_index1), getElement(_index2), getElement(_index3));
	}

	template <typename T>
	bool ComputeVector3Base<T>::isEqual(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isLess(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxLess(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxLess(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxLess(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isLessOrEqual(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxLessOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxLessOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxLessOrEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isGreater(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxGreater(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxGreater(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxGreater(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isGreaterOrEqual(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxGreaterOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) &&
			math::IsApproxGreaterOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) &&
			math::IsApproxGreaterOrEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isAnyLess(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxLess(m_vector.m_x, _vector.m_vector.m_x, _epsilon) ||
			math::IsApproxLess(m_vector.m_y, _vector.m_vector.m_y, _epsilon) ||
			math::IsApproxLess(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isAnyLessOrEqual(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxLessOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) ||
			math::IsApproxLessOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) ||
			math::IsApproxLessOrEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isAnyGreater(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxGreater(m_vector.m_x, _vector.m_vector.m_x, _epsilon) ||
			math::IsApproxGreater(m_vector.m_y, _vector.m_vector.m_y, _epsilon) ||
			math::IsApproxGreater(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isAnyGreaterOrEqual(const ComputeVector3Base<T>& _vector, T _epsilon) const
	{
		return
			math::IsApproxGreaterOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) ||
			math::IsApproxGreaterOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon) ||
			math::IsApproxGreaterOrEqual(m_vector.m_z, _vector.m_vector.m_z, _epsilon);
	}

	template <typename T>
	bool ComputeVector3Base<T>::isUnit() const
	{
		return math::IsApproxEqual(getLengthSqr(), T(1.0));
	}

	template <typename T>
	bool ComputeVector3Base<T>::isZero() const
	{
		return math::IsApproxEqual(getLengthSqr(), T(0.0));
	}

	template <typename T>
	ComputeVector3Base<T> operator+(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		ComputeVector3Base<T> result(_vector1);
		return result += _vector2;
	}

	template <typename T>
	ComputeVector3Base<T> operator-(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		ComputeVector3Base<T> result(_vector1);
		return result -= _vector2;
	}

	template <typename T>
	ComputeVector3Base<T> operator-(const ComputeVector3Base<T>& _vector)
	{
		ComputeVector3Base<T> result(_vector);
		return result.negate();
	}

	template <typename T>
	ComputeVector3Base<T> operator*(const ComputeVector3Base<T>& _vector, T _value)
	{
		ComputeVector3Base<T> result(_vector);
		return result *= _value;
	}

	template <typename T>
	ComputeVector3Base<T> operator*(T _value, const ComputeVector3Base<T>& _vector)
	{
		return operator*(_vector, _value);
	}

	template <typename T>
	ComputeVector3Base<T> operator/(const ComputeVector3Base<T>& _vector, T _value)
	{
		ComputeVector3Base<T> result(_vector);
		return result /= _value;
	}


	template <typename T>
	ComputeVector3Base<T> operator/(T _value, const ComputeVector3Base<T>& _vector)
	{
		return operator/(_vector, _value);
	}

	template <typename T>
	T DotComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		ComputeVector3Base<T> result(_vector1);
		return result.dot(_vector2);
	}

	template <typename T>
	ComputeVector3Base<T> CrossComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		return _vector1.cross(_vector2);
	}

	template <typename T>
	ComputeVector3Base<T> NegateComputeVector3(const ComputeVector3Base<T>& _vector)
	{
		ComputeVector3Base<T> result(_vector);
		return result.negate();
	}

	template <typename T>
	ComputeVector3Base<T> ComputeReciprocalComputeVector3(const ComputeVector3Base<T>& _vector)
	{
		ComputeVector3Base<T> result(_vector);
		return result.computeReciprocal();
	}

	template <typename T>
	ComputeVector3Base<T> NormalizeComputeVector3(const ComputeVector3Base<T>& _vector)
	{
		ComputeVector3Base<T> result(_vector);
		return result.normalize();
	}

	template <typename T>
	T LengthComputeVector3(const ComputeVector3Base<T>& _vector)
	{
		return _vector.getLength();
	}

	template <typename T>
	T LengthSqrComputeVector3(const ComputeVector3Base<T>& _vector)
	{
		return _vector.getLengthSqr();
	}

	template <typename T>
	ComputeVector3Base<T> MultiplyComputeVector3Elements(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		ComputeVector3Base<T> result(_vector1);
		return result.multiplyElements(_vector2);
	}

	template <typename T>
	ComputeVector3Base<T> MinComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		return ComputeVector3Base<T>(
			std::min(_vector1.getX(), _vector2.getX()),
			std::min(_vector1.getY(), _vector2.getY()),
			std::min(_vector1.getZ(), _vector2.getZ())
		);
	}

	template <typename T>
	ComputeVector3Base<T> MaxComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
	{
		return ComputeVector3Base<T>(
			std::max(_vector1.getX(), _vector2.getX()),
			std::max(_vector1.getY(), _vector2.getY()),
			std::max(_vector1.getZ(), _vector2.getZ())
		);
	}
}