#pragma once

#include <type_traits>
#include <vector>

namespace ego
{
	template <typename T>
	class DynamicVectorBase final
	{
	public:
		using ValueType = T;
		using ValueContainer = std::vector<ValueType>;

		static_assert(std::is_arithmetic_v<ValueType>);

		static constexpr ValueType DefaultValue = 0;

	private:
		ValueContainer m_values;

	public:
		DynamicVectorBase() = default;
		DynamicVectorBase(size_t _dimension);
		DynamicVectorBase(ValueContainer&& _values);
		DynamicVectorBase(const DynamicVectorBase& _vector);
		DynamicVectorBase(DynamicVectorBase&& _vector);

		DynamicVectorBase& operator=(const DynamicVectorBase& _vector);
		DynamicVectorBase& operator=(DynamicVectorBase&& _vector);
		ValueType operator[](size_t _index) const;
		ValueType& operator[](size_t _index);

		ValueType getElement(size_t _index) const;
		ValueType& getElement(size_t _index);

		void setElement(size_t _index, ValueType _value);

		void reset();

		uint32_t getElementCount() const;
	};

	using FloatDynamicVector = DynamicVectorBase<float>;
}

#include "DynamicVector.hpp"