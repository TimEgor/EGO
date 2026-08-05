#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "ComputeMath.h"

namespace ego
{
    template <typename T>
    class DynamicVectorBase final
    {
    public:
        using ValueType = T;
        using ValueContainer = std::vector<ValueType>;
        using ValueView = std::span<ValueType>;
        using ConstValueView = std::span<const ValueType>;

        static_assert(std::is_arithmetic_v<ValueType> && !std::is_same_v<ValueType, bool>);

        static constexpr ValueType DefaultValue = 0;

    private:
        ValueContainer m_values;

        static bool IsValidElementCount(size_t _elementCount);

    public:
        DynamicVectorBase() = default;
        explicit DynamicVectorBase(size_t _dimension);
        explicit DynamicVectorBase(ValueContainer&& _values);
        explicit DynamicVectorBase(ValueView _values);
        explicit DynamicVectorBase(ConstValueView _values);
        DynamicVectorBase(const DynamicVectorBase& _vector) = default;
        DynamicVectorBase(DynamicVectorBase&& _vector) noexcept = default;

        DynamicVectorBase& operator=(const DynamicVectorBase& _vector) = default;
        DynamicVectorBase& operator=(DynamicVectorBase&& _vector) noexcept = default;
        ValueType operator[](size_t _index) const;
        ValueType& operator[](size_t _index);

        ValueView getValues();
        ConstValueView getValues() const;

        ValueType getElement(size_t _index) const;
        ValueType& getElement(size_t _index);

        void setElement(size_t _index, ValueType _value);

        void reset();

        uint32_t getElementCount() const;
    };

    using DynamicVector = DynamicVectorBase<ComputeValue>;
    using FloatDynamicVector = DynamicVectorBase<float>;
} // namespace ego

#include "DynamicVector.hpp"
