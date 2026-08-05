#pragma once

#include "EgoCore/Assert/Assert.h"

namespace ego
{
#pragma region Vector2Base
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
    constexpr typename Vector2Base<T>::ValueType Vector2Base<T>::getX() const
    {
        return m_x;
    }

    template <typename T>
    constexpr void Vector2Base<T>::setX(ValueType _x)
    {
        m_x = _x;
    }

    template <typename T>
    constexpr typename Vector2Base<T>::ValueType Vector2Base<T>::getY() const
    {
        return m_y;
    }

    template <typename T>
    constexpr void Vector2Base<T>::setY(ValueType _y)
    {
        m_y = _y;
    }

    template <typename T>
    typename Vector2Base<T>::ValueType Vector2Base<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < 2);

        if (_index == 0)
        {
            return m_x;
        }

        return m_y;
    }

    template <typename T>
    typename Vector2Base<T>::ValueType& Vector2Base<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < 2);

        if (_index == 0)
        {
            return m_x;
        }

        return m_y;
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
        m_z = DefaultValue;

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
    constexpr typename Vector3Base<T>::ValueType Vector3Base<T>::getX() const
    {
        return m_x;
    }

    template <typename T>
    constexpr void Vector3Base<T>::setX(ValueType _x)
    {
        m_x = _x;
    }

    template <typename T>
    constexpr typename Vector3Base<T>::ValueType Vector3Base<T>::getY() const
    {
        return m_y;
    }

    template <typename T>
    constexpr void Vector3Base<T>::setY(ValueType _y)
    {
        m_y = _y;
    }

    template <typename T>
    constexpr typename Vector3Base<T>::ValueType Vector3Base<T>::getZ() const
    {
        return m_z;
    }

    template <typename T>
    constexpr void Vector3Base<T>::setZ(ValueType _z)
    {
        m_z = _z;
    }

    template <typename T>
    typename Vector3Base<T>::ValueType Vector3Base<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < 3);

        if (_index == 0)
        {
            return m_x;
        }

        if (_index == 1)
        {
            return m_y;
        }

        return m_z;
    }

    template <typename T>
    typename Vector3Base<T>::ValueType& Vector3Base<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < 3);

        if (_index == 0)
        {
            return m_x;
        }

        if (_index == 1)
        {
            return m_y;
        }

        return m_z;
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
    constexpr typename Vector4Base<T>::ValueType Vector4Base<T>::getX() const
    {
        return m_x;
    }

    template <typename T>
    constexpr void Vector4Base<T>::setX(ValueType _x)
    {
        m_x = _x;
    }

    template <typename T>
    constexpr typename Vector4Base<T>::ValueType Vector4Base<T>::getY() const
    {
        return m_y;
    }

    template <typename T>
    constexpr void Vector4Base<T>::setY(ValueType _y)
    {
        m_y = _y;
    }

    template <typename T>
    constexpr typename Vector4Base<T>::ValueType Vector4Base<T>::getZ() const
    {
        return m_z;
    }

    template <typename T>
    constexpr void Vector4Base<T>::setZ(ValueType _z)
    {
        m_z = _z;
    }

    template <typename T>
    constexpr typename Vector4Base<T>::ValueType Vector4Base<T>::getW() const
    {
        return m_w;
    }

    template <typename T>
    constexpr void Vector4Base<T>::setW(ValueType _w)
    {
        m_w = _w;
    }

    template <typename T>
    typename Vector4Base<T>::ValueType Vector4Base<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < 4);

        if (_index == 0)
        {
            return m_x;
        }

        if (_index == 1)
        {
            return m_y;
        }

        if (_index == 2)
        {
            return m_z;
        }

        return m_w;
    }

    template <typename T>
    typename Vector4Base<T>::ValueType& Vector4Base<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < 4);

        if (_index == 0)
        {
            return m_x;
        }

        if (_index == 1)
        {
            return m_y;
        }

        if (_index == 2)
        {
            return m_z;
        }

        return m_w;
    }

    template <typename T>
    void Vector4Base<T>::setElement(size_t _index, ValueType _value)
    {
        ValueType& element = getElement(_index);
        element = _value;
    }
#pragma endregion
} // namespace ego
