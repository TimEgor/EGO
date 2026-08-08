#pragma once

#include <optional>

#include <magic_enum/magic_enum.hpp>

namespace ego::rtti
{
    template <typename Enum>
    requires std::is_enum_v<Enum>
    TypedEnumPropertyValue<Enum>::TypedEnumPropertyValue(void* _object, const TypedEnumPropertyMetaInfo<Enum>& _propertyMetaInfo)
        : EnumPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    TypedEnumPropertyValue<Enum>::TypedEnumPropertyValue(const void* _object, const TypedEnumPropertyMetaInfo<Enum>& _propertyMetaInfo)
        : EnumPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    const TypedEnumPropertyMetaInfo<Enum>& TypedEnumPropertyValue<Enum>::getMetaInfo() const
    {
        return static_cast<const TypedEnumPropertyMetaInfo<Enum>&>(PropertyValue::getMetaInfo());
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    size_t TypedEnumPropertyValue<Enum>::getValueIndex() const
    {
        const std::optional<size_t> valueIndex = magic_enum::enum_index(getValue<Enum>());

        return valueIndex.value_or(InvalidEnumValueIndex);
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    bool TypedEnumPropertyValue<Enum>::setValueIndex(size_t _index)
    {
        static constexpr auto Values = magic_enum::enum_values<Enum>();
        Enum* value = PropertyValue::tryGetMutableValue<Enum>();
        if (_index >= Values.size() || !value)
        {
            return false;
        }

        *value = Values[_index];

        return true;
    }
} // namespace ego::rtti
