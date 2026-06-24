namespace ego
{
    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::operator=(const ComputeVector2Base& _vector)
    {
        m_vector = _vector.m_vector;

        return *this;
    }

    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::operator+=(const ComputeVector2Base& _vector)
    {
        m_vector.m_x += _vector.m_vector.m_x;
        m_vector.m_y += _vector.m_vector.m_y;

        return *this;
    }

    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::operator-=(const ComputeVector2Base& _vector)
    {
        m_vector.m_x -= _vector.m_vector.m_x;
        m_vector.m_y -= _vector.m_vector.m_y;

        return *this;
    }

    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::operator*=(T _value)
    {
        m_vector.m_x *= _value;
        m_vector.m_y *= _value;

        return *this;
    }

    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::operator/=(T _value)
    {
        EGO_ASSERT(std::abs(_value) > math::TypedEpsilon<T>());

        m_vector.m_x /= _value;
        m_vector.m_y /= _value;

        return *this;
    }

    template <typename T>
    bool ComputeVector2Base<T>::operator==(const ComputeVector2Base<T>& _vector) const
    {
        return isEqual(_vector);
    }

    template <typename T>
    bool ComputeVector2Base<T>::operator!=(const ComputeVector2Base<T>& _vector) const
    {
        return !operator==(_vector);
    }

    template <typename T>
    FloatVector2 ComputeVector2Base<T>::getFloatVector2() const
    {
        FloatVector2 result;
        getFloatVector2(result);

        return result;
    }

    template <typename T>
    void ComputeVector2Base<T>::getFloatVector2(FloatVector2& _out) const
    {
        _out = FloatVector2(float(m_vector.m_x), float(m_vector.m_y));
    }

    template <typename T>
    T ComputeVector2Base<T>::getLength() const
    {
        return sqrt(getLengthSqr());
    }

    template <typename T>
    T ComputeVector2Base<T>::getLengthSqr() const
    {
        return dot(*this);
    }

    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::negate()
    {
        m_vector.m_x = -m_vector.m_x;
        m_vector.m_y = -m_vector.m_y;

        return *this;
    }

    template <typename T>
    ComputeVector2Base<T> ComputeVector2Base<T>::getUnit() const
    {
        ComputeVector2Base<T> result(*this);
        return result.normalize();
    }

    template <typename T>
    ComputeVector2Base<T>& ComputeVector2Base<T>::normalize()
    {
        const T length = getLength();
        if (length <= math::TypedEpsilon<T>())
        {
            return *this;
        }

        return operator/=(length);
    }

    template <typename T>
    T ComputeVector2Base<T>::dot(const ComputeVector2Base& _vector) const
    {
        return m_vector.m_x * _vector.m_vector.m_x + m_vector.m_y * _vector.m_vector.m_y;
    }

    template <typename T>
    bool ComputeVector2Base<T>::isEqual(const ComputeVector2Base& _vector, T _epsilon) const
    {
        return math::IsApproxEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) && math::IsApproxEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon);
    }

    template <typename T>
    bool ComputeVector2Base<T>::isLess(const ComputeVector2Base& _vector, T _epsilon) const
    {
        return math::IsApproxLess(m_vector.m_x, _vector.m_vector.m_x, _epsilon) && math::IsApproxLess(m_vector.m_y, _vector.m_vector.m_y, _epsilon);
    }

    template <typename T>
    bool ComputeVector2Base<T>::isLessOrEqual(const ComputeVector2Base& _vector, T _epsilon) const
    {
        return math::IsApproxLessOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) && math::IsApproxLessOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon);
    }

    template <typename T>
    bool ComputeVector2Base<T>::isGreater(const ComputeVector2Base& _vector, T _epsilon) const
    {
        return math::IsApproxGreater(m_vector.m_x, _vector.m_vector.m_x, _epsilon) && math::IsApproxGreater(m_vector.m_y, _vector.m_vector.m_y, _epsilon);
    }

    template <typename T>
    bool ComputeVector2Base<T>::isGreaterOrEqual(const ComputeVector2Base& _vector, T _epsilon) const
    {
        return math::IsApproxGreaterOrEqual(m_vector.m_x, _vector.m_vector.m_x, _epsilon) && math::IsApproxGreaterOrEqual(m_vector.m_y, _vector.m_vector.m_y, _epsilon);
    }

    template <typename T>
    bool ComputeVector2Base<T>::isUnit() const
    {
        return math::IsApproxEqual(getLengthSqr(), T(1.0));
    }

    template <typename T>
    bool ComputeVector2Base<T>::isZero() const
    {
        return math::IsApproxEqual(getLengthSqr(), T(0.0));
    }

    template <typename T>
    ComputeVector2Base<T> operator+(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2)
    {
        ComputeVector2Base<T> result(_vector1);
        return result += _vector2;
    }

    template <typename T>
    ComputeVector2Base<T> operator-(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2)
    {
        ComputeVector2Base<T> result(_vector1);
        return result -= _vector2;
    }

    template <typename T>
    ComputeVector2Base<T> operator-(const ComputeVector2Base<T>& _vector)
    {
        ComputeVector2Base<T> result(_vector);
        return result.negate();
    }

    template <typename T>
    ComputeVector2Base<T> operator*(const ComputeVector2Base<T>& _vector, T _value)
    {
        ComputeVector2Base<T> result(_vector);
        return result *= _value;
    }

    template <typename T>
    ComputeVector2Base<T> operator*(T _value, const ComputeVector2Base<T>& _vector)
    {
        return operator*(_vector, _value);
    }

    template <typename T>
    ComputeVector2Base<T> operator/(const ComputeVector2Base<T>& _vector, T _value)
    {
        ComputeVector2Base<T> result(_vector);
        return result /= _value;
    }

    template <typename T>
    ComputeVector2Base<T> operator/(T _value, const ComputeVector2Base<T>& _vector)
    {
        return operator/(_vector, _value);
    }

    template <typename T>
    T DotComputeVector2(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2)
    {
        ComputeVector2Base<T> result(_vector1);
        return result.dot(_vector2);
    }
} // namespace ego
