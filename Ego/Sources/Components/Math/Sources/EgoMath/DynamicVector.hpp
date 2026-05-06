#pragma once

#include "EgoCore/Assert/AssertCore.h"

namespace ego
{
#pragma region DynamicVectorBase
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
	DynamicVectorBase<T>::DynamicVectorBase(const DynamicVectorBase& _vector)
		: m_values(_vector.m_values)
	{
		reset();
	}

	template <typename T>
	DynamicVectorBase<T>::DynamicVectorBase(DynamicVectorBase&& _vector)
		: m_values(std::move(_vector.m_values))
	{
	}

	template <typename T>
	DynamicVectorBase<T>& DynamicVectorBase<T>::operator=(const DynamicVectorBase& _vector)
	{
		m_values = _vector.m_values;
		return *this;
	}

	template <typename T>
	DynamicVectorBase<T>& DynamicVectorBase<T>::operator=(DynamicVectorBase&& _vector)
	{
		m_values = std::move(_vector.m_values);
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
		//EGO_ASSERT(_index < m_values.size());
		return m_values[_index];
	}

	template <typename T>
	typename DynamicVectorBase<T>::ValueType& DynamicVectorBase<T>::getElement(size_t _index)
	{
		//EGO_ASSERT(_index < m_values.size());
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
		return m_values.size();
	}

#pragma endregion
}
