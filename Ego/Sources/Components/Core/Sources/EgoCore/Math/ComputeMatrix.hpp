#pragma once

namespace ego
{
    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(ValueType _m11, ValueType _m12, ValueType _m21, ValueType _m22)
        requires(Size == 2)
        : m_columns{ColumnVectorType(_m11, _m12), ColumnVectorType(_m21, _m22)}
    {
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(
        ValueType _m11,
        ValueType _m12,
        ValueType _m13,
        ValueType _m21,
        ValueType _m22,
        ValueType _m23,
        ValueType _m31,
        ValueType _m32,
        ValueType _m33)
        requires(Size == 3)
        : m_columns{ColumnVectorType(_m11, _m12, _m13), ColumnVectorType(_m21, _m22, _m23), ColumnVectorType(_m31, _m32, _m33)}
    {
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(
        ValueType _m11,
        ValueType _m12,
        ValueType _m13,
        ValueType _m14,
        ValueType _m21,
        ValueType _m22,
        ValueType _m23,
        ValueType _m24,
        ValueType _m31,
        ValueType _m32,
        ValueType _m33,
        ValueType _m34,
        ValueType _m41,
        ValueType _m42,
        ValueType _m43,
        ValueType _m44)
        requires(Size == 4)
        : m_columns{
              ColumnVectorType(_m11, _m12, _m13, _m14),
              ColumnVectorType(_m21, _m22, _m23, _m24),
              ColumnVectorType(_m31, _m32, _m33, _m34),
              ColumnVectorType(_m41, _m42, _m43, _m44)}
    {
    }

    template <typename T, uint32_t Size>
    template <typename SourceValueType>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const Matrix2x2Base<SourceValueType>& _matrix)
        requires(Size == 2)
        : m_columns{}
    {
        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            for (uint32_t rowIndex = 0; rowIndex < Size; ++rowIndex)
            {
                m_columns[columnIndex].setElement(rowIndex, static_cast<ValueType>(_matrix.m_values[columnIndex][rowIndex]));
            }
        }
    }

    template <typename T, uint32_t Size>
    template <typename SourceValueType>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const Matrix3x3Base<SourceValueType>& _matrix)
        requires(Size == 3)
        : m_columns{}
    {
        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            for (uint32_t rowIndex = 0; rowIndex < Size; ++rowIndex)
            {
                m_columns[columnIndex].setElement(rowIndex, static_cast<ValueType>(_matrix.m_values[columnIndex][rowIndex]));
            }
        }
    }

    template <typename T, uint32_t Size>
    template <typename SourceValueType>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const Matrix4x4Base<SourceValueType>& _matrix)
        requires(Size == 4)
        : m_columns{}
    {
        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            for (uint32_t rowIndex = 0; rowIndex < Size; ++rowIndex)
            {
                m_columns[columnIndex].setElement(rowIndex, static_cast<ValueType>(_matrix.m_values[columnIndex][rowIndex]));
            }
        }
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const ColumnVectorType& _column1, const ColumnVectorType& _column2)
        requires(Size == 2)
        : m_columns{_column1, _column2}
    {
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const ColumnVectorType& _column1, const ColumnVectorType& _column2, const ColumnVectorType& _column3)
        requires(Size == 3)
        : m_columns{_column1, _column2, _column3}
    {
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(
        const ColumnVectorType& _column1,
        const ColumnVectorType& _column2,
        const ColumnVectorType& _column3,
        const ColumnVectorType& _column4)
        requires(Size == 4)
        : m_columns{_column1, _column2, _column3, _column4}
    {
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const ComputeMatrixBase<ValueType, 2>& _matrix)
        requires(Size == 3 || Size == 4)
        : m_columns{}
    {
        m_columns[0] = ColumnVectorType(_matrix.m_columns[0]);
        m_columns[1] = ColumnVectorType(_matrix.m_columns[1]);

        for (uint32_t columnIndex = 2; columnIndex < Size; ++columnIndex)
        {
            m_columns[columnIndex] = ColumnVectorType(ValueType(0.0));
        }
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>::ComputeMatrixBase(const ComputeMatrixBase<ValueType, 3>& _matrix)
        requires(Size == 4)
        : m_columns{ColumnVectorType(_matrix.m_columns[0]), ColumnVectorType(_matrix.m_columns[1]), ColumnVectorType(_matrix.m_columns[2]), ColumnVectorType(ValueType(0.0))}
    {
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::operator+=(const ComputeMatrixBase& _matrix)
    {
        return math::simd::AddMatrixElements(*this, _matrix, Size);
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::operator-=(const ComputeMatrixBase& _matrix)
    {
        return math::simd::SubtractMatrixElements(*this, _matrix, Size);
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::operator*=(const ComputeMatrixBase& _matrix)
    {
        return math::simd::MultiplyMatrixElements(*this, _matrix, Size);
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::operator*=(ValueType _value)
    {
        return math::simd::MultiplyMatrixElements(*this, _value, Size);
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::operator/=(ValueType _value)
    {
        return math::simd::DivideMatrixElements(*this, _value, Size);
    }

    template <typename T, uint32_t Size>
    bool ComputeMatrixBase<T, Size>::operator==(const ComputeMatrixBase& _matrix) const
    {
        return isEqual(_matrix);
    }

    template <typename T, uint32_t Size>
    bool ComputeMatrixBase<T, Size>::operator!=(const ComputeMatrixBase& _matrix) const
    {
        return !isEqual(_matrix);
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::MatrixType ComputeMatrixBase<T, Size>::getMatrix() const
    {
        return getMatrix<ValueType>();
    }

    template <typename T, uint32_t Size>
    void ComputeMatrixBase<T, Size>::getMatrix(MatrixType& _out) const
    {
        getMatrix<ValueType>(_out);
    }

    template <typename T, uint32_t Size>
    template <typename ResultValueType>
    typename ComputeMatrixStorageTraits<ResultValueType, Size>::StorageType ComputeMatrixBase<T, Size>::getMatrix() const
    {
        typename ComputeMatrixStorageTraits<ResultValueType, Size>::StorageType result;
        getMatrix<ResultValueType>(result);

        return result;
    }

    template <typename T, uint32_t Size>
    template <typename ResultValueType>
    void ComputeMatrixBase<T, Size>::getMatrix(typename ComputeMatrixStorageTraits<ResultValueType, Size>::StorageType& _out) const
    {
        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            for (uint32_t rowIndex = 0; rowIndex < Size; ++rowIndex)
            {
                _out.setElement(columnIndex, rowIndex, static_cast<ResultValueType>(getStoredElement(columnIndex, rowIndex)));
            }
        }
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::ValueType ComputeMatrixBase<T, Size>::getElement(uint32_t _row, uint32_t _column) const
    {
        return getStoredElement(_column, _row);
    }

    template <typename T, uint32_t Size>
    void ComputeMatrixBase<T, Size>::setElement(uint32_t _row, uint32_t _column, ValueType _value)
    {
        setStoredElement(_column, _row, _value);
    }

    template <typename T, uint32_t Size>
    const typename ComputeMatrixBase<T, Size>::ColumnVectorType& ComputeMatrixBase<T, Size>::getColumn(uint32_t _index) const
    {
        EGO_ASSERT(_index < Size);
        return m_columns[_index];
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::ColumnVectorType& ComputeMatrixBase<T, Size>::getColumn(uint32_t _index)
    {
        EGO_ASSERT(_index < Size);
        return m_columns[_index];
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::ColumnVectorType ComputeMatrixBase<T, Size>::getRow(uint32_t _index) const
    {
        EGO_ASSERT(_index < Size);

        ColumnVectorType result;
        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            result.setElement(columnIndex, getStoredElement(columnIndex, _index));
        }

        return result;
    }

    template <typename T, uint32_t Size>
    void ComputeMatrixBase<T, Size>::setColumn(uint32_t _index, const ColumnVectorType& _column)
    {
        EGO_ASSERT(_index < Size);
        m_columns[_index] = _column;
    }

    template <typename T, uint32_t Size>
    void ComputeMatrixBase<T, Size>::setRow(uint32_t _index, const ColumnVectorType& _row)
    {
        EGO_ASSERT(_index < Size);

        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            setStoredElement(columnIndex, _index, _row.getElement(columnIndex));
        }
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::negate()
    {
        return math::simd::NegateMatrixElements(*this, Size);
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::transpose()
    {
        ComputeMatrixBase transposed;

        for (uint32_t columnIndex = 0; columnIndex < Size; ++columnIndex)
        {
            transposed.setColumn(columnIndex, getRow(columnIndex));
        }

        *this = transposed;
        return *this;
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::ValueType ComputeMatrixBase<T, Size>::getDeterminant() const
    {
        if constexpr (Size == 2)
        {
            return getStoredElement(0, 0) * getStoredElement(1, 1) - getStoredElement(1, 0) * getStoredElement(0, 1);
        }
        else if constexpr (Size == 3)
        {
            return getStoredElement(0, 0) * (getStoredElement(1, 1) * getStoredElement(2, 2) - getStoredElement(2, 1) * getStoredElement(1, 2)) -
                   getStoredElement(0, 1) * (getStoredElement(1, 0) * getStoredElement(2, 2) - getStoredElement(2, 0) * getStoredElement(1, 2)) +
                   getStoredElement(0, 2) * (getStoredElement(1, 0) * getStoredElement(2, 1) - getStoredElement(2, 0) * getStoredElement(1, 1));
        }
        else
        {
            const ValueType m00 = getStoredElement(0, 0);
            const ValueType m01 = getStoredElement(0, 1);
            const ValueType m02 = getStoredElement(0, 2);
            const ValueType m03 = getStoredElement(0, 3);
            const ValueType m10 = getStoredElement(1, 0);
            const ValueType m11 = getStoredElement(1, 1);
            const ValueType m12 = getStoredElement(1, 2);
            const ValueType m13 = getStoredElement(1, 3);
            const ValueType m20 = getStoredElement(2, 0);
            const ValueType m21 = getStoredElement(2, 1);
            const ValueType m22 = getStoredElement(2, 2);
            const ValueType m23 = getStoredElement(2, 3);
            const ValueType m30 = getStoredElement(3, 0);
            const ValueType m31 = getStoredElement(3, 1);
            const ValueType m32 = getStoredElement(3, 2);
            const ValueType m33 = getStoredElement(3, 3);

            const ValueType det0 = m11 * (m22 * m33 - m32 * m23) - m21 * (m12 * m33 - m32 * m13) + m31 * (m12 * m23 - m22 * m13);
            const ValueType det1 = m01 * (m22 * m33 - m32 * m23) - m21 * (m02 * m33 - m32 * m03) + m31 * (m02 * m23 - m22 * m03);
            const ValueType det2 = m01 * (m12 * m33 - m32 * m13) - m11 * (m02 * m33 - m32 * m03) + m31 * (m02 * m13 - m12 * m03);
            const ValueType det3 = m01 * (m12 * m23 - m22 * m13) - m11 * (m02 * m23 - m22 * m03) + m21 * (m02 * m13 - m12 * m03);

            return m00 * det0 - m10 * det1 + m20 * det2 - m30 * det3;
        }
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size>& ComputeMatrixBase<T, Size>::invert()
    {
        const ValueType determinant = getDeterminant();
        EGO_ASSERT(std::abs(determinant) > math::TypedEpsilon<ValueType>());

        if constexpr (Size == 2)
        {
            ComputeMatrixBase inverted(getStoredElement(1, 1), -getStoredElement(0, 1), -getStoredElement(1, 0), getStoredElement(0, 0));
            inverted *= ValueType(1.0) / determinant;

            *this = inverted;
        }
        else if constexpr (Size == 3)
        {
            ColumnVectorType column0(
                (getStoredElement(1, 1) * getStoredElement(2, 2) - getStoredElement(2, 1) * getStoredElement(1, 2)),
                -(getStoredElement(0, 1) * getStoredElement(2, 2) - getStoredElement(2, 1) * getStoredElement(0, 2)),
                (getStoredElement(0, 1) * getStoredElement(1, 2) - getStoredElement(0, 2) * getStoredElement(1, 1)));

            ColumnVectorType column1(
                -(getStoredElement(1, 0) * getStoredElement(2, 2) - getStoredElement(2, 0) * getStoredElement(1, 2)),
                (getStoredElement(0, 0) * getStoredElement(2, 2) - getStoredElement(2, 0) * getStoredElement(0, 2)),
                -(getStoredElement(0, 0) * getStoredElement(1, 2) - getStoredElement(1, 0) * getStoredElement(0, 2)));

            ColumnVectorType column2(
                (getStoredElement(1, 0) * getStoredElement(2, 1) - getStoredElement(2, 0) * getStoredElement(1, 1)),
                -(getStoredElement(0, 0) * getStoredElement(2, 1) - getStoredElement(2, 0) * getStoredElement(0, 1)),
                (getStoredElement(0, 0) * getStoredElement(1, 1) - getStoredElement(0, 1) * getStoredElement(1, 0)));

            ComputeMatrixBase inverted(column0, column1, column2);
            inverted *= ValueType(1.0) / determinant;

            *this = inverted;
        }
        else
        {
            const ValueType m00 = getStoredElement(0, 0);
            const ValueType m01 = getStoredElement(0, 1);
            const ValueType m02 = getStoredElement(0, 2);
            const ValueType m03 = getStoredElement(0, 3);
            const ValueType m10 = getStoredElement(1, 0);
            const ValueType m11 = getStoredElement(1, 1);
            const ValueType m12 = getStoredElement(1, 2);
            const ValueType m13 = getStoredElement(1, 3);
            const ValueType m20 = getStoredElement(2, 0);
            const ValueType m21 = getStoredElement(2, 1);
            const ValueType m22 = getStoredElement(2, 2);
            const ValueType m23 = getStoredElement(2, 3);
            const ValueType m30 = getStoredElement(3, 0);
            const ValueType m31 = getStoredElement(3, 1);
            const ValueType m32 = getStoredElement(3, 2);
            const ValueType m33 = getStoredElement(3, 3);

            const ValueType s0 = m00 * m11 - m10 * m01;
            const ValueType s1 = m00 * m12 - m10 * m02;
            const ValueType s2 = m00 * m13 - m10 * m03;
            const ValueType s3 = m01 * m12 - m11 * m02;
            const ValueType s4 = m01 * m13 - m11 * m03;
            const ValueType s5 = m02 * m13 - m12 * m03;

            const ValueType c5 = m22 * m33 - m32 * m23;
            const ValueType c4 = m21 * m33 - m31 * m23;
            const ValueType c3 = m21 * m32 - m31 * m22;
            const ValueType c2 = m20 * m33 - m30 * m23;
            const ValueType c1 = m20 * m32 - m30 * m22;
            const ValueType c0 = m20 * m31 - m30 * m21;

            ColumnVectorType column0(m11 * c5 - m12 * c4 + m13 * c3, -m01 * c5 + m02 * c4 - m03 * c3, m31 * s5 - m32 * s4 + m33 * s3, -m21 * s5 + m22 * s4 - m23 * s3);
            ColumnVectorType column1(-m10 * c5 + m12 * c2 - m13 * c1, m00 * c5 - m02 * c2 + m03 * c1, -m30 * s5 + m32 * s2 - m33 * s1, m20 * s5 - m22 * s2 + m23 * s1);
            ColumnVectorType column2(m10 * c4 - m11 * c2 + m13 * c0, -m00 * c4 + m01 * c2 - m03 * c0, m30 * s4 - m31 * s2 + m33 * s0, -m20 * s4 + m21 * s2 - m23 * s0);
            ColumnVectorType column3(-m10 * c3 + m11 * c1 - m12 * c0, m00 * c3 - m01 * c1 + m02 * c0, -m30 * s3 + m31 * s1 - m32 * s0, m20 * s3 - m21 * s1 + m22 * s0);

            ComputeMatrixBase inverted(column0, column1, column2, column3);
            inverted *= ValueType(1.0) / determinant;

            *this = inverted;
        }

        return *this;
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::ColumnVectorType ComputeMatrixBase<T, Size>::transform(const ColumnVectorType& _vector) const
    {
        ColumnVectorType result;
        transform(_vector, result);

        return result;
    }

    template <typename T, uint32_t Size>
    void ComputeMatrixBase<T, Size>::transform(const ColumnVectorType& _vector, ColumnVectorType& _out) const
    {
        if constexpr (Size == 2)
        {
            math::simd::TransformMatrix2x2VectorElements(m_columns[0], m_columns[1], _vector, _out);
        }
        else if constexpr (Size == 3)
        {
            math::simd::TransformMatrix3x3VectorElements(m_columns[0], m_columns[1], m_columns[2], _vector, _out);
        }
        else
        {
            math::simd::TransformMatrix4x4VectorElements(m_columns[0], m_columns[1], m_columns[2], m_columns[3], _vector, _out);
        }
    }

    template <typename T, uint32_t Size>
    bool ComputeMatrixBase<T, Size>::isEqual(const ComputeMatrixBase& _matrix, ValueType _epsilon) const
    {
        return math::simd::AreMatrixElementsEqual(*this, _matrix, _epsilon, Size);
    }

    template <typename T, uint32_t Size>
    typename ComputeMatrixBase<T, Size>::ValueType ComputeMatrixBase<T, Size>::getStoredElement(uint32_t _column, uint32_t _row) const
    {
        EGO_ASSERT(_column < Size);
        EGO_ASSERT(_row < Size);
        return m_columns[_column].getElement(_row);
    }

    template <typename T, uint32_t Size>
    void ComputeMatrixBase<T, Size>::setStoredElement(uint32_t _column, uint32_t _row, ValueType _value)
    {
        EGO_ASSERT(_column < Size);
        EGO_ASSERT(_row < Size);
        m_columns[_column].setElement(_row, _value);
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator+(const ComputeMatrixBase<T, Size>& _matrix1, const ComputeMatrixBase<T, Size>& _matrix2)
    {
        ComputeMatrixBase<T, Size> result(_matrix1);
        return result += _matrix2;
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator-(const ComputeMatrixBase<T, Size>& _matrix1, const ComputeMatrixBase<T, Size>& _matrix2)
    {
        ComputeMatrixBase<T, Size> result(_matrix1);
        return result -= _matrix2;
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator-(const ComputeMatrixBase<T, Size>& _matrix)
    {
        ComputeMatrixBase<T, Size> result(_matrix);
        return result.negate();
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator*(const ComputeMatrixBase<T, Size>& _matrix1, const ComputeMatrixBase<T, Size>& _matrix2)
    {
        ComputeMatrixBase<T, Size> result(_matrix1);
        return result *= _matrix2;
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator*(const ComputeMatrixBase<T, Size>& _matrix, T _value)
    {
        ComputeMatrixBase<T, Size> result(_matrix);
        return result *= _value;
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator*(T _value, const ComputeMatrixBase<T, Size>& _matrix)
    {
        ComputeMatrixBase<T, Size> result(_matrix);
        return result *= _value;
    }

    template <typename T, uint32_t Size>
    ComputeMatrixBase<T, Size> operator/(const ComputeMatrixBase<T, Size>& _matrix, T _value)
    {
        ComputeMatrixBase<T, Size> result(_matrix);
        return result /= _value;
    }

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator*(const ComputeMatrixBase<T, Size>& _matrix, const ComputeVectorBase<T, Size>& _vector)
    {
        return _matrix.transform(_vector);
    }
} // namespace ego
