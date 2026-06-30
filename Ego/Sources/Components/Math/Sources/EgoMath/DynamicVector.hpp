#pragma once

#include "EgoCore/Assert/AssertCore.h"

namespace ego
{
#pragma region DynamicVectorBase
    template <typename T>
    bool DynamicVectorBase<T>::isView() const
    {
        return !m_view.empty();
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueView DynamicVectorBase<T>::getValues()
    {
        return isView() ? m_view : ValueView(m_values);
    }

    template <typename T>
    typename DynamicVectorBase<T>::ConstValueView DynamicVectorBase<T>::getValues() const
    {
        return isView() ? ConstValueView(m_view) : ConstValueView(m_values);
    }

    template <typename T>
    void DynamicVectorBase<T>::assignValues(ConstValueView _values)
    {
        ValueView values = getValues();
        EGO_ASSERT(values.size() == _values.size());
        std::copy(_values.begin(), _values.end(), values.begin());
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(size_t _dimension)
        : m_values(_dimension)
    {
        EGO_ASSERT(_dimension > 0);
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(ValueContainer&& _values)
        : m_values(std::move(_values))
    {
        EGO_ASSERT(getElementCount() > 0);
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(ValueView _values)
        : m_view(_values)
    {
        EGO_ASSERT(getElementCount() > 0);
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(const DynamicVectorBase& _vector)
    {
        ConstValueView values = _vector.getValues();
        m_values.assign(values.begin(), values.end());
    }

    template <typename T>
    DynamicVectorBase<T>::DynamicVectorBase(DynamicVectorBase&& _vector)
        : m_values(std::move(_vector.m_values)),
          m_view(_vector.m_view)
    {
        if (!_vector.isView())
        {
            _vector.m_view = ValueView();
        }
    }

    template <typename T>
    DynamicVectorBase<T>& DynamicVectorBase<T>::operator=(const DynamicVectorBase& _vector)
    {
        if (isView())
        {
            assignValues(_vector.getValues());
        }
        else
        {
            ConstValueView values = _vector.getValues();
            m_values.assign(values.begin(), values.end());
        }

        return *this;
    }

    template <typename T>
    DynamicVectorBase<T>& DynamicVectorBase<T>::operator=(DynamicVectorBase&& _vector)
    {
        if (isView())
        {
            assignValues(_vector.getValues());
        }
        else if (_vector.isView())
        {
            ConstValueView values = _vector.getValues();
            m_values.assign(values.begin(), values.end());
        }
        else
        {
            m_values = std::move(_vector.m_values);
            m_view = ValueView();
        }

        return *this;
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
    typename DynamicVectorBase<T>::ValueType DynamicVectorBase<T>::getElement(size_t _index) const
    {
        ConstValueView values = getValues();
        EGO_ASSERT(_index < values.size());
        return values[_index];
    }

    template <typename T>
    typename DynamicVectorBase<T>::ValueType& DynamicVectorBase<T>::getElement(size_t _index)
    {
        ValueView values = getValues();
        EGO_ASSERT(_index < values.size());
        return values[_index];
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
        ValueView values = getValues();
        std::fill(values.begin(), values.end(), DefaultValue);
    }

    template <typename T>
    uint32_t DynamicVectorBase<T>::getElementCount() const
    {
        const size_t elementCount = getValues().size();
        EGO_ASSERT(elementCount <= std::numeric_limits<uint32_t>::max());
        return static_cast<uint32_t>(elementCount);
    }

#pragma endregion
} // namespace ego
