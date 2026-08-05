#pragma once

namespace ego
{
    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase()
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(ValueType(0.0), ValueType(0.0), ValueType(0.0), ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(ValueType _value)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_value, _value, Size >= 3 ? _value : ValueType(0.0), Size == 4 ? _value : ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(ValueType _x, ValueType _y) requires(Size == 2)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_x, _y, ValueType(0.0), ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(ValueType _x, ValueType _y, ValueType _z) requires(Size == 3)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_x, _y, _z, ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(ValueType _x, ValueType _y, ValueType _z, ValueType _w) requires(Size == 4)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_x, _y, _z, _w))
    {
    }

    template <typename T, uint32_t Size>
    template <typename SourceValueType>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const Vector2Base<SourceValueType>& _vector) requires(Size == 2)
        : ComputeVectorBase()
    {
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            setElement(elementIndex, static_cast<ValueType>(_vector.getElement(elementIndex)));
        }
    }

    template <typename T, uint32_t Size>
    template <typename SourceValueType>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const Vector3Base<SourceValueType>& _vector) requires(Size == 3)
        : ComputeVectorBase()
    {
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            setElement(elementIndex, static_cast<ValueType>(_vector.getElement(elementIndex)));
        }
    }

    template <typename T, uint32_t Size>
    template <typename SourceValueType>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const Vector4Base<SourceValueType>& _vector) requires(Size == 4)
        : ComputeVectorBase()
    {
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            setElement(elementIndex, static_cast<ValueType>(_vector.getElement(elementIndex)));
        }
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector, ValueType _z) requires(Size == 3)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_vector.getX(), _vector.getY(), _z, ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector1, const ComputeVectorBase<ValueType, 2>& _vector2)
        requires(Size == 4)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_vector1.getX(), _vector1.getY(), _vector2.getX(), _vector2.getY()))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector, ValueType _z, ValueType _w) requires(Size == 4)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_vector.getX(), _vector.getY(), _z, _w))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector) requires(Size == 3 || Size == 4)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_vector.getX(), _vector.getY(), ValueType(0.0), ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const ComputeVectorBase<ValueType, 3>& _vector) requires(Size == 4)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_vector.getX(), _vector.getY(), _vector.getZ(), ValueType(0.0)))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>::ComputeVectorBase(const ComputeVectorBase<ValueType, 3>& _vector, ValueType _w) requires(Size == 4)
        : m_nativeValues(math::simd::MakeNativeComputeVectorStorage(_vector.getX(), _vector.getY(), _vector.getZ(), _w))
    {
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::operator+=(const ComputeVectorBase& _vector)
    {
        return math::simd::AddVectorElements(*this, _vector, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::operator-=(const ComputeVectorBase& _vector)
    {
        return math::simd::SubtractVectorElements(*this, _vector, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::operator*=(ValueType _value)
    {
        return math::simd::MultiplyVectorElements(*this, _value, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::operator/=(ValueType _value)
    {
        return math::simd::DivideVectorElements(*this, _value, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::operator==(const ComputeVectorBase& _vector) const
    {
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            if (getElement(elementIndex) != _vector.getElement(elementIndex))
            {
                return false;
            }
        }

        return true;
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::operator!=(const ComputeVectorBase& _vector) const
    {
        return !operator==(_vector);
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::operator[](uint32_t _index) const
    {
        return getElement(_index);
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getElement(uint32_t _index) const
    {
        EGO_ASSERT(_index < Size);
        return math::simd::GetNativeComputeVectorElement(m_nativeValues, _index);
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setElement(uint32_t _index, ValueType _value)
    {
        EGO_ASSERT(_index < Size);
        math::simd::SetNativeComputeVectorElement(m_nativeValues, _index, _value);
    }

    template <typename T, uint32_t Size>
    const typename ComputeVectorBase<T, Size>::NativeVectorType& ComputeVectorBase<T, Size>::getNativeValues() const
    {
        return m_nativeValues;
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::NativeVectorType& ComputeVectorBase<T, Size>::getNativeValues()
    {
        return m_nativeValues;
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setNativeValues(const NativeVectorType& _values)
    {
        m_nativeValues = _values;
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::VectorType ComputeVectorBase<T, Size>::getVector() const
    {
        return getVector<ValueType>();
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::getVector(VectorType& _out) const
    {
        getVector<ValueType>(_out);
    }

    template <typename T, uint32_t Size>
    template <typename ResultValueType>
    typename ComputeVectorStorageTraits<ResultValueType, Size>::StorageType ComputeVectorBase<T, Size>::getVector() const
    {
        typename ComputeVectorStorageTraits<ResultValueType, Size>::StorageType result;
        getVector<ResultValueType>(result);

        return result;
    }

    template <typename T, uint32_t Size>
    template <typename ResultValueType>
    void ComputeVectorBase<T, Size>::getVector(typename ComputeVectorStorageTraits<ResultValueType, Size>::StorageType& _out) const
    {
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            _out.setElement(elementIndex, static_cast<ResultValueType>(getElement(elementIndex)));
        }
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getX() const
    {
        return getElement(0);
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getY() const
    {
        return getElement(1);
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getZ() const requires(Size >= 3)
    {
        return getElement(2);
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getW() const requires(Size == 4)
    {
        return getElement(3);
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setX(ValueType _value)
    {
        setElement(0, _value);
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setY(ValueType _value)
    {
        setElement(1, _value);
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setZ(ValueType _value) requires(Size >= 3)
    {
        setElement(2, _value);
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setW(ValueType _value) requires(Size == 4)
    {
        setElement(3, _value);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<typename ComputeVectorBase<T, Size>::ValueType, 2> ComputeVectorBase<T, Size>::getXY() const requires(Size >= 3)
    {
        return ComputeVectorBase<ValueType, 2>(getX(), getY());
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<typename ComputeVectorBase<T, Size>::ValueType, 2> ComputeVectorBase<T, Size>::getZW() const requires(Size == 4)
    {
        return ComputeVectorBase<ValueType, 2>(getZ(), getW());
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<typename ComputeVectorBase<T, Size>::ValueType, 3> ComputeVectorBase<T, Size>::getXYZ() const requires(Size == 4)
    {
        return ComputeVectorBase<ValueType, 3>(getX(), getY(), getZ());
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setXY(const ComputeVectorBase<ValueType, 2>& _vector) requires(Size >= 3)
    {
        setX(_vector.getX());
        setY(_vector.getY());
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setZW(const ComputeVectorBase<ValueType, 2>& _vector) requires(Size == 4)
    {
        setZ(_vector.getX());
        setW(_vector.getY());
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::setXYZ(const ComputeVectorBase<ValueType, 3>& _vector) requires(Size == 4)
    {
        setX(_vector.getX());
        setY(_vector.getY());
        setZ(_vector.getZ());
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getLength() const
    {
        return sqrt(getLengthSqr());
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::getLengthSqr() const
    {
        return dot(*this);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::negate()
    {
        return math::simd::NegateVectorElements(*this, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::computeReciprocal() requires(Size == 3)
    {
        return math::simd::ReciprocalVectorElements(*this, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::multiplyElements(const ComputeVectorBase& _vector) requires(Size == 3)
    {
        return math::simd::MultiplyVectorElements(*this, _vector, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> ComputeVectorBase<T, Size>::getUnit() const
    {
        ComputeVectorBase result(*this);
        return result.normalize();
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::getUnit(ComputeVectorBase& _out) const requires(Size == 3)
    {
        _out = getUnit();
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size>& ComputeVectorBase<T, Size>::normalize()
    {
        const ValueType length = getLength();
        if (length <= math::TypedEpsilon<ValueType>())
        {
            return *this;
        }

        return operator/=(length);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> ComputeVectorBase<T, Size>::getOrthogonal() const requires(Size == 3)
    {
        ComputeVectorBase result;
        getOrthogonal(result);

        return result;
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::getOrthogonal(ComputeVectorBase& _out) const requires(Size == 3)
    {
        if (abs(getX()) > abs(getY()))
        {
            _out = ComputeVectorBase(getZ(), ValueType(0.0), -getX());
        }
        else
        {
            _out = ComputeVectorBase(ValueType(0.0), getZ(), -getY());
        }
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> ComputeVectorBase<T, Size>::getAbs() const requires(Size == 3)
    {
        ComputeVectorBase result;
        getAbs(result);

        return result;
    }

    template <typename T, uint32_t Size>
    void ComputeVectorBase<T, Size>::getAbs(ComputeVectorBase& _out) const requires(Size == 3)
    {
        _out = *this;
        math::simd::AbsVectorElements(_out, Size);
    }

    template <typename T, uint32_t Size>
    uint32_t ComputeVectorBase<T, Size>::getMinElementIndex() const requires(Size == 3)
    {
        return getX() < getY() ? getX() < getZ() ? 0 : 2 : getY() < getZ() ? 1 : 2;
    }

    template <typename T, uint32_t Size>
    uint32_t ComputeVectorBase<T, Size>::getMaxElementIndex() const requires(Size == 3)
    {
        return getX() > getY() ? getX() > getZ() ? 0 : 2 : getY() > getZ() ? 1 : 2;
    }

    template <typename T, uint32_t Size>
    typename ComputeVectorBase<T, Size>::ValueType ComputeVectorBase<T, Size>::dot(const ComputeVectorBase& _vector) const
    {
        return math::simd::DotVectorElements(*this, _vector, Size);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> ComputeVectorBase<T, Size>::cross(const ComputeVectorBase& _vector) const requires(Size == 3)
    {
        ComputeVectorBase result;
        math::simd::CrossVector3Elements(*this, _vector, result);

        return result;
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> ComputeVectorBase<T, Size>::swizzle(uint32_t _index1, uint32_t _index2, uint32_t _index3) const requires(Size == 3)
    {
        return ComputeVectorBase(getElement(_index1), getElement(_index2), getElement(_index3));
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isEqual(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::AreVectorElementsEqual(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isLess(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::AreVectorElementsLess(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isLessOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::AreVectorElementsLessOrEqual(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isGreater(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::AreVectorElementsGreater(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isGreaterOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::AreVectorElementsGreaterOrEqual(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isAnyLess(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::IsAnyVectorElementLess(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isAnyLessOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::IsAnyVectorElementLessOrEqual(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isAnyGreater(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::IsAnyVectorElementGreater(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isAnyGreaterOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon) const
    {
        return math::simd::IsAnyVectorElementGreaterOrEqual(*this, _vector, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isUnit() const
    {
        return math::IsApproxEqual(getLengthSqr(), ValueType(1.0));
    }

    template <typename T, uint32_t Size>
    bool ComputeVectorBase<T, Size>::isZero() const
    {
        return math::IsApproxEqual(getLengthSqr(), ValueType(0.0));
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator+(const ComputeVectorBase<T, Size>& _vector1, const ComputeVectorBase<T, Size>& _vector2)
    {
        ComputeVectorBase<T, Size> result(_vector1);
        return result += _vector2;
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator-(const ComputeVectorBase<T, Size>& _vector1, const ComputeVectorBase<T, Size>& _vector2)
    {
        ComputeVectorBase<T, Size> result(_vector1);
        return result -= _vector2;
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator-(const ComputeVectorBase<T, Size>& _vector)
    {
        ComputeVectorBase<T, Size> result(_vector);
        return result.negate();
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator*(const ComputeVectorBase<T, Size>& _vector, T _value)
    {
        ComputeVectorBase<T, Size> result(_vector);
        return result *= _value;
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator*(T _value, const ComputeVectorBase<T, Size>& _vector)
    {
        return operator*(_vector, _value);
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator/(const ComputeVectorBase<T, Size>& _vector, T _value)
    {
        ComputeVectorBase<T, Size> result(_vector);
        return result /= _value;
    }
} // namespace ego
