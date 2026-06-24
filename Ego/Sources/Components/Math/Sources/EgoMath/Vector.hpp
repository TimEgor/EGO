#pragma once

namespace ego
{
#pragma region Vector2Base
    template <typename T>
    Vector2Base<T>& Vector2Base<T>::operator=(const Vector2Base& _vector)
    {
        m_x = _vector.m_x;
        m_y = _vector.m_y;

        return *this;
    }

    template <typename T>
    typename Vector2Base<T>::ValueType Vector2Base<T>::operator[](size_t _index) const
    {
        return getElement(_index);
    }

    template <typename T>
    typename Vector2Base<T>::ValueType& Vector2Base<T>::operator[](size_t _index)
    {
        return getElement(_index);
    }

    template <typename T>
    typename Vector2Base<T>::ValueType Vector2Base<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < 2);
        return m_values[_index];
    }

    template <typename T>
    typename Vector2Base<T>::ValueType& Vector2Base<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < 2);
        return m_values[_index];
    }

    template <typename T>
    void Vector2Base<T>::setElement(size_t _index, ValueType _value)
    {
        ValueType& element = getElement(_index);
        element = _value;
    }
#pragma endregion

#pragma region Vector3Base
    template <typename T>
    Vector3Base<T>& Vector3Base<T>::operator=(const Vector2Base<ValueType>& _vector)
    {
        m_x = _vector.m_x;
        m_y = _vector.m_y;
        m_z = _vector.DefaultValue;

        return *this;
    }

    template <typename T>
    Vector3Base<T>& Vector3Base<T>::operator=(const Vector3Base& _vector)
    {
        m_x = _vector.m_x;
        m_y = _vector.m_y;
        m_z = _vector.m_z;

        return *this;
    }

    template <typename T>
    typename Vector3Base<T>::ValueType Vector3Base<T>::operator[](size_t _index) const
    {
        return getElement(_index);
    }

    template <typename T>
    typename Vector3Base<T>::ValueType& Vector3Base<T>::operator[](size_t _index)
    {
        return getElement(_index);
    }

    template <typename T>
    typename Vector3Base<T>::ValueType Vector3Base<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < 3);
        return m_values[_index];
    }

    template <typename T>
    typename Vector3Base<T>::ValueType& Vector3Base<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < 3);
        return m_values[_index];
    }

    template <typename T>
    void Vector3Base<T>::setElement(size_t _index, ValueType _value)
    {
        ValueType& element = getElement(_index);
        element = _value;
    }
#pragma endregion

#pragma region Vector4Base
    template <typename T>
    Vector4Base<T>& Vector4Base<T>::operator=(const Vector2Base<ValueType>& _vector)
    {
        m_x = _vector.m_x;
        m_y = _vector.m_y;
        m_z = DefaultValue;
        m_w = DefaultValue;

        return *this;
    }

    template <typename T>
    Vector4Base<T>& Vector4Base<T>::operator=(const Vector3Base<ValueType>& _vector)
    {
        m_x = _vector.m_x;
        m_y = _vector.m_y;
        m_z = _vector.m_z;
        m_w = DefaultValue;

        return *this;
    }

    template <typename T>
    Vector4Base<T>& Vector4Base<T>::operator=(const Vector4Base& _vector)
    {
        m_x = _vector.m_x;
        m_y = _vector.m_y;
        m_z = _vector.m_z;
        m_w = _vector.m_w;

        return *this;
    }

    template <typename T>
    typename Vector4Base<T>::ValueType Vector4Base<T>::operator[](size_t _index) const
    {
        return getElement(_index);
    }

    template <typename T>
    typename Vector4Base<T>::ValueType& Vector4Base<T>::operator[](size_t _index)
    {
        return getElement(_index);
    }

    template <typename T>
    typename Vector4Base<T>::ValueType Vector4Base<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < 4);
        return m_values[_index];
    }

    template <typename T>
    typename Vector4Base<T>::ValueType& Vector4Base<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < 4);
        return m_values[_index];
    }

    template <typename T>
    void Vector4Base<T>::setElement(size_t _index, ValueType _value)
    {
        ValueType& element = getElement(_index);
        element = _value;
    }
#pragma endregion
} // namespace ego
