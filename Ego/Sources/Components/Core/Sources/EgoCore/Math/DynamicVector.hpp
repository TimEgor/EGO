#pragma once

#include "EgoCore/Assert/Assert.h"

namespace ego
{
#pragma region DynamicVectorBase
    template <typename T>
    bool DynamicVectorBase<T>::IsValidElementCount(size_t _elementCount)
    {
        return _elementCount > 0 && _elementCount <= std::numeric_limits<uint32_t>::max();
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(size_t _dimension)
    {
        if (!IsValidElementCount(_dimension))
        {
            EGO_ASSERT(_dimension > 0 && _dimension <= std::numeric_limits<uint32_t>::max());

            return;
        }

        m_values.resize(_dimension, DefaultValue);
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(ValueContainer&& _values)
    {
        if (!IsValidElementCount(_values.size()))
        {
            EGO_ASSERT(_values.size() > 0 && _values.size() <= std::numeric_limits<uint32_t>::max());

            return;
        }

        m_values = std::move(_values);
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(ValueView _values)
        : DynamicVectorBase(ConstValueView(_values))
    {
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(ConstValueView _values)
    {
        if (!IsValidElementCount(_values.size()))
        {
            EGO_ASSERT(_values.size() > 0 && _values.size() <= std::numeric_limits<uint32_t>::max());

            return;
        }

        m_values.assign(_values.begin(), _values.end());
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueType DynamicVectorBase<T>::operator[](size_t _index) const
    {
        return getElement(_index);
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueType& DynamicVectorBase<T>::operator[](size_t _index)
    {
        return getElement(_index);
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueView DynamicVectorBase<T>::getValues()
    {
        return ValueView(m_values);
    }

    template <typename T>
    typename DynamicVectorBase<T>::ConstValueView DynamicVectorBase<T>::getValues() const
    {
        return ConstValueView(m_values);
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueType DynamicVectorBase<T>::getElement(size_t _index) const
    {
        EGO_ASSERT(_index < m_values.size());
        return m_values[_index];
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueType& DynamicVectorBase<T>::getElement(size_t _index)
    {
        EGO_ASSERT(_index < m_values.size());
        return m_values[_index];
    }

    template <typename T>
    void DynamicVectorBase<T>::setElement(size_t _index, ValueType _value)
    {
        ValueType& element = getElement(_index);
        element = _value;
    }

    template <typename T>
    void DynamicVectorBase<T>::reset()
    {
        std::fill(m_values.begin(), m_values.end(), DefaultValue);
    }

    template <typename T>
    uint32_t DynamicVectorBase<T>::getElementCount() const
    {
        EGO_ASSERT(m_values.size() <= std::numeric_limits<uint32_t>::max());
        return static_cast<uint32_t>(m_values.size());
    }

#pragma endregion
} // namespace ego
